#pragma once
#include <memory>
#include <unordered_set>
#include "Epoch/Assets/Asset.h"

namespace Epoch
{
	class AssetManagerBase
	{
	public:
		AssetManagerBase() = default;
		virtual ~AssetManagerBase() = default;

		virtual AssetType GetAssetType(AssetHandle aHandle) = 0;
		virtual std::shared_ptr<Asset> GetAsset(AssetHandle aHandle) = 0;
		virtual std::shared_ptr<Asset> GetAssetAsync(AssetHandle aHandle) = 0;
		virtual void AddMemoryOnlyAsset(std::shared_ptr<Asset> aAsset, const std::string& aName) = 0;
		virtual bool ReloadData(AssetHandle assetHandle) = 0;
		virtual void RemoveAsset(AssetHandle handle) = 0;
		virtual bool IsMemoryAsset(AssetHandle handle) = 0;
		virtual bool IsAssetHandleValid(AssetHandle aHandle) = 0;
		virtual bool IsAssetLoaded(AssetHandle aHandle) = 0;
		virtual bool IsAssetValid(AssetHandle handle) = 0;
		virtual bool IsAssetMissing(AssetHandle handle) = 0;

		virtual std::unordered_set<AssetHandle> GetAllAssetsWithType(AssetType aType) = 0;
		virtual const std::unordered_map<AssetHandle, std::shared_ptr<Asset>>& GetLoadedAssets() = 0;
	};
}
