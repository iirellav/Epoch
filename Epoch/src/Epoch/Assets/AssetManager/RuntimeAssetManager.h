#pragma once
#include "AssetManagerBase.h"
#include "Epoch/Assets/AssetPack/AssetPack.h"

namespace Epoch
{
	class RuntimeAssetManager : public AssetManagerBase
	{
	public:
		RuntimeAssetManager();
		~RuntimeAssetManager() override = default;

		void LoadBuiltInAssets() override;

		AssetType GetAssetType(AssetHandle aHandle) override;
		std::shared_ptr<Asset> GetAsset(AssetHandle aHandle) override;
		std::shared_ptr<Asset> GetAssetAsync(AssetHandle aHandle) override;
		void AddMemoryOnlyAsset(std::shared_ptr<Asset> aAsset) override;
		void AddMemoryOnlyAsset(std::shared_ptr<Asset> aAsset, const std::string& aName) override;

		bool ReloadData(AssetHandle aAssetHandle) override;
		void RemoveAsset(AssetHandle aHandle) override;

		bool IsMemoryAsset(AssetHandle aHandle) override;
		bool IsAssetHandleValid(AssetHandle aHandle) override;
		bool IsAssetLoaded(AssetHandle aHandle) override;

		std::unordered_set<AssetHandle> GetAllAssetsWithType(AssetType aType) override;
		const std::unordered_map<AssetHandle, std::shared_ptr<Asset>>& GetLoadedAssets() override;


		// Loads Scene and makes active
		std::shared_ptr<Scene> LoadScene(AssetHandle aHandle);

		void SetAssetPack(std::shared_ptr<AssetPack> aAssetPack) { myAssetPack = aAssetPack; }

	private:
		AssetMap myLoadedAssets;
		AssetMap myMemoryAssets;

		// TODO: Support multiple asset packs.
		std::shared_ptr<AssetPack> myAssetPack;
		AssetHandle myActiveScene = 0;
	};
}
