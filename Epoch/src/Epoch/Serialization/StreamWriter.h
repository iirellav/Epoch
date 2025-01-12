#pragma once
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
	};
}
