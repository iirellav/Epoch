#pragma once
#include "StreamWriter.h"
#include "StreamReader.h"

namespace Epoch
{
	class BufferStreamWriter : public StreamWriter
	{
	public:
		BufferStreamWriter(Buffer& aBuffer, size_t aSize);
		BufferStreamWriter(const BufferStreamWriter&) = delete;
		~BufferStreamWriter() = default;

		bool IsStreamGood() const override { return myWritePos < myBuffer.size; }
		uint64_t GetStreamPosition() override { return myWritePos; }
		void SetStreamPosition(uint64_t aPosition) override { myWritePos = aPosition; }
		bool WriteData(const char* aData, size_t aSize) override;

	private:
		Buffer& myBuffer;
		size_t myWritePos = 0;
	};

	class BufferStreamReader : public StreamReader
	{
	public:
		BufferStreamReader(const Buffer& aBuffer);
		BufferStreamReader(const BufferStreamReader&) = delete;
		~BufferStreamReader() = default;

		bool IsStreamGood() const override { return myReadPos < myBuffer.size; }
		uint64_t GetStreamPosition() override { return myReadPos; }
		void SetStreamPosition(uint64_t aPosition) override { myReadPos = aPosition; }
		bool ReadData(char* aDestination, size_t aSize) override;

	private:
		const Buffer& myBuffer;
		size_t myReadPos = 0;
	};
}
