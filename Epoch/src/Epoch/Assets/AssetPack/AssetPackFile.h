#pragma once
#include <map>
#include "Epoch/Assets/Asset.h"

namespace Epoch
{
	struct AssetPackFile
	{
		struct AssetInfo
		{
			uint64_t packedOffset;
			uint64_t packedSize;
			uint16_t type;
		};

		struct SceneInfo
		{
			uint64_t packedOffset = 0;
			uint64_t packedSize = 0;
			std::map<AssetHandle, AssetInfo> assets;
		};

		struct IndexTable
		{
			uint64_t packedAppBinaryOffset = 0;
			uint64_t packedAppBinarySize = 0;
			std::map<AssetHandle, SceneInfo> scenes;
		};

		struct FileHeader
		{
			const char HEADER[4] = { 'E','P','A','P' };
			uint32_t version = 1;
			uint64_t buildVersion = 0;
		};

		FileHeader header;
		IndexTable indexTable;
	};
}
