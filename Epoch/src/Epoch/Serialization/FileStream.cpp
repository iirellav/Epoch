#include "epch.h"
#include "FileStream.h"

namespace Epoch
{
	//FileStreamWriter
	FileStreamWriter::FileStreamWriter(const std::filesystem::path& aPath) : myPath(aPath)
	{
		myStream = std::ofstream(aPath, std::ifstream::out | std::ifstream::binary);
	}

	FileStreamWriter::~FileStreamWriter()
	{
		myStream.close();
	}

	bool FileStreamWriter::WriteData(const char* aData, size_t aSize)
	{
		myStream.write(aData, aSize);
		return true;
	}

	//FileStreamReader
	FileStreamReader::FileStreamReader(const std::filesystem::path& aPath) : myPath(aPath)
	{
		myStream = std::ifstream(aPath, std::ifstream::in | std::ifstream::binary);
	}

	FileStreamReader::~FileStreamReader()
	{
		myStream.close();
	}

	bool FileStreamReader::ReadData(char* aDestination, size_t aSize)
	{
		myStream.read(aDestination, aSize);
		return true;
	}

}