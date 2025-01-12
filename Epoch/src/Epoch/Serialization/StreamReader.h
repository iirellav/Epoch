#pragma once
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
	};
}
