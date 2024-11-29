#pragma once
#include "Epoch/Assets/AssetSerializer/AssetSerializer.h"

namespace Epoch
{
	class AssetImporter
	{
	public:
		static void Init();
		static void Serialize(const AssetMetadata& aMetadata, const std::shared_ptr<Asset>& aAsset);
		static void Serialize(const std::shared_ptr<Asset>& aAsset);
		static bool TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset);

	private:
		inline static std::unordered_map<AssetType, std::unique_ptr<AssetSerializer>> staticSerializers;
	};
}
