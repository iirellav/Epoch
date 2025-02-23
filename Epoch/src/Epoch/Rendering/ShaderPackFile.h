#pragma once
#include <map>
#include <vector>
#include <Epoch/Serialization/StreamReader.h>
#include <Epoch/Serialization/StreamWriter.h>

namespace Epoch
{
	struct ShaderPackFile
	{
		struct ShaderData
		{
			uint8_t stage;
			void* data;
		};

		struct ShaderModuleInfo
		{
			uint64_t packedOffset;
			uint64_t packedSize; // size of data only
			uint8_t version;
			uint8_t stage;
			uint32_t flags = 0;

			static void Serialize(StreamWriter* aWriter, const ShaderModuleInfo& aInfo) { aWriter->WriteRaw(aInfo); }
			static void Deserialize(StreamReader* aReader, ShaderModuleInfo& aInfo) { aReader->ReadRaw(aInfo); }
		};

		struct ShaderProgramInfo
		{
			std::vector<uint32_t> moduleIndices;
		};

		struct IndexTable
		{
			std::map<uint32_t, ShaderProgramInfo> shaderPrograms; // Hashed shader name/path
			std::vector<ShaderModuleInfo> shaderModules;
		};

		struct FileHeader
		{
			char HEADER[4] = { 'E','P','S','P' };
			uint32_t version = 1;
			uint32_t shaderProgramCount;
			uint32_t shaderModuleCount;
		};

		FileHeader header;
		IndexTable indexTable;
		ShaderData* data;
	};
}
