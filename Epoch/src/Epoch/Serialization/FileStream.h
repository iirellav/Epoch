#pragma once
#include <filesystem>
#include <fstream>
#include "StreamWriter.h"
#include "StreamReader.h"

namespace Epoch
{
	class FileStreamWriter : public StreamWriter
	{
	public:
		FileStreamWriter(const std::filesystem::path& aPath);
		FileStreamWriter(const FileStreamWriter&) = delete;
		virtual ~FileStreamWriter();

		bool IsStreamGood() const override { return myStream.good(); }
		uint64_t GetStreamPosition() override { return myStream.tellp(); }
		void SetStreamPosition(uint64_t aPosition) override { myStream.seekp(aPosition); }
		bool WriteData(const char* aData, size_t aSize) override;

	private:
		std::filesystem::path myPath;
		std::ofstream myStream;
	};

	class FileStreamReader : public StreamReader
	{
	public:
		FileStreamReader(const std::filesystem::path& aPath);
		FileStreamReader(const FileStreamReader&) = delete;
		~FileStreamReader();

		bool IsStreamGood() const override { return myStream.good(); }
		uint64_t GetStreamPosition() override { return myStream.tellg(); }
		void SetStreamPosition(uint64_t aPosition) override { myStream.seekg(aPosition); }
		bool ReadData(char* aDestination, size_t aSize) override;

	private:
		std::filesystem::path myPath;
		std::ifstream myStream;
	};
}
