#include "epch.h"
#include "StreamWriter.h"

namespace Epoch
{
	void StreamWriter::WriteBuffer(Buffer aBuffer, bool aWriteSize)
	{
		if (aWriteSize)
		{
			WriteData((char*)&aBuffer.size, sizeof(uint32_t));
		}

		WriteData((char*)aBuffer.data, aBuffer.size);
	}

	void StreamWriter::WriteZero(uint64_t aSize)
	{
		char zero = 0;
		for (uint64_t i = 0; i < aSize; i++)
		{
			WriteData(&zero, 1);
		}
	}

	void StreamWriter::WriteString(const std::string& aString)
	{
		size_t size = aString.size();
		WriteData((char*)&size, sizeof(size_t));
		WriteData((char*)aString.data(), sizeof(char) * size);
	}
}