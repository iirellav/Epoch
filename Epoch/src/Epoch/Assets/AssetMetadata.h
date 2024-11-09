#pragma once
#include <filesystem>
#include "Asset.h"

namespace Epoch
{
	struct AssetMetadata
	{
		AssetHandle handle = 0;
		AssetType type = AssetType::None;

		std::filesystem::path filePath = "";
		bool isDataLoaded = false;
		bool isMemoryAsset = false;

		bool IsValid() const { return type != AssetType::None && !isMemoryAsset; }
	};
}
