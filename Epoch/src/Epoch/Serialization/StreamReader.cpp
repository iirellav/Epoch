#include "epch.h"
#include "StreamReader.h"

namespace Epoch
{
	void StreamReader::ReadBuffer(Buffer& aBuffer, uint32_t aSize)
	{
		aBuffer.size = aSize;
		if (aSize == 0)
		{
			ReadData((char*)&aBuffer.size, sizeof(uint32_t));
		}

		aBuffer.Allocate(aBuffer.size);
		ReadData((char*)aBuffer.data, aBuffer.size);
	}

	void StreamReader::ReadString(std::string& aString)
	{
		size_t size;
		ReadData((char*)&size, sizeof(size_t));

		aString.resize(size);
		ReadData((char*)aString.data(), sizeof(char) * size);
	}
}
