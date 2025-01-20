#pragma once
#include <map>
#include <vector>
#include "Epoch/Core/Buffer.h"

namespace Epoch
{
	class StreamWriter
	{
	public:
		StreamWriter() = default;
		~StreamWriter() = default;

		virtual bool IsStreamGood() const = 0;
		virtual uint64_t GetStreamPosition() = 0;
		virtual void SetStreamPosition(uint64_t aPosition) = 0;
		virtual bool WriteData(const char* aData, size_t aSize) = 0;

		operator bool() const { return IsStreamGood(); }

		void WriteBuffer(Buffer aBuffer, bool aWriteSize = true);
		void WriteZero(uint64_t aSize);
		void WriteString(const std::string& aString);

		template<typename T>
		void WriteRaw(const T& type)
		{
			bool success = WriteData((char*)&type, sizeof(T));
			EPOCH_ASSERT(success, "Stream writer failed to write raw!");
		}

		template<typename T>
		void WriteObject(const T& aObject)
		{
			T::Serialize(this, aObject);
		}

		template<typename Key, typename Value>
		void WriteMap(const std::map<Key, Value>& aMap, bool aWriteSize = true)
		{
			if (aWriteSize)
			{
				WriteRaw<uint32_t>((uint32_t)aMap.size());
			}

			for (const auto& [key, value] : aMap)
			{
				if constexpr (std::is_trivial<Key>())
				{
					WriteRaw<Key>(key);
				}
				else
				{
					WriteObject<Key>(key);
				}

				if constexpr (std::is_trivial<Value>())
				{
					WriteRaw<Value>(value);
				}
				else
				{
					WriteObject<Value>(value);
				}
			}
		}

		template<typename T>
		void WriteArray(const std::vector<T>& aArray, bool aWriteSize = true)
		{
			if (aWriteSize)
			{
				WriteRaw<uint32_t>((uint32_t)aArray.size());
			}

			for (const auto& element : aArray)
			{
				if constexpr (std::is_trivial<T>())
				{
					WriteRaw<T>(element);
				}
				else
				{
					WriteObject<T>(element);
				}
			}
		}
	};
}
