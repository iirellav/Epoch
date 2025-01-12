#include "epch.h"
#include "BufferStream.h"

namespace Epoch
{
    //StreamWriter
    BufferStreamWriter::BufferStreamWriter(Buffer& aBuffer, size_t aSize) : myBuffer(aBuffer)
    {
        if (aSize > aBuffer.size)
        {
            aBuffer.Allocate((uint32_t)aSize);
        }
    }

    bool BufferStreamWriter::WriteData(const char* aData, size_t aSize)
    {
        if (myWritePos + aSize > myBuffer.size)
        {
            return false;
        }

        myBuffer.Write(aData, (uint32_t)aSize, (uint32_t)myWritePos);
        return true;
    }

    //StreamReader
    BufferStreamReader::BufferStreamReader(const Buffer& aBuffer) : myBuffer(aBuffer) {}

    bool BufferStreamReader::ReadData(char* aDestination, size_t aSize)
    {
        if (myReadPos + aSize > myBuffer.size)
        {
            return false;
        }

        memcpy(aDestination, (char*)myBuffer.data + myReadPos, aSize);
        return true;
    }
}
