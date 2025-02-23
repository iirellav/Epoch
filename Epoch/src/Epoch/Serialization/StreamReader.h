#pragma once
#include <map>
#include <vector>
#include "Epoch/Core/Buffer.h"

namespace Epoch
{
	class StreamReader
	{
	public:
		StreamReader() = default;
		~StreamReader() = default;

		virtual bool IsStreamGood() const = 0;
		virtual uint64_t GetStreamPosition() = 0;
		virtual void SetStreamPosition(uint64_t aPosition) = 0;
		virtual bool ReadData(char* aDestination, size_t aSize) = 0;

		operator bool() const { return IsStreamGood(); }

		void ReadBuffer(Buffer& aBuffer, uint32_t aSize = 0);
		void ReadString(std::string& aString);

		template<typename T>
		void ReadRaw(T& type)
		{
			bool success = ReadData((char*)&type, sizeof(T));
			EPOCH_ASSERT(success, "Stream reader failed to read raw!");
		}

		template<typename T>
		void ReadObject(T& aObject)
		{
			T::Deserialize(this, aObject);
		}

		template<typename Key, typename Value>
		void ReadMap(std::map<Key, Value>& aMap, uint32_t aSize = 0)
		{
			if (aSize == 0)
			{
				ReadRaw<uint32_t>(aSize);
			}

			for (uint32_t i = 0; i < aSize; i++)
			{
				Key key;
				if constexpr (std::is_trivial<Key>())
				{
					ReadRaw<Key>(key);
				}
				else
				{
					ReadObject<Key>(key);
				}

				if constexpr (std::is_trivial<Value>())
				{
					ReadRaw<Value>(aMap[key]);
				}
				else
				{
					ReadObject<Value>(aMap[key]);
				}
			}
		}

		template<typename T>
		void ReadArray(std::vector<T>& aArray, uint32_t aSize = 0)
		{
			if (aSize == 0)
			{
				ReadRaw<uint32_t>(aSize);
			}

			aArray.resize(aSize);

			for (uint32_t i = 0; i < aSize; i++)
			{
				if constexpr (std::is_trivial<T>())
				{
					ReadRaw<T>(aArray[i]);
				}
				else
				{
					ReadObject<T>(aArray[i]);
				}
			}
		}
	};
}
