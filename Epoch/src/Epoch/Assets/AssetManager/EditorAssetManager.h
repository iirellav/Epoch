#pragma once
#include <string>
#include <future>
#include <filesystem>
#include "AssetManagerBase.h"
#include "Epoch/Assets/AssetRegistry.h"
#include "Epoch/Assets/AssetImporter.h"

namespace Epoch
{
	using AssetMap = std::unordered_map<AssetHandle, std::shared_ptr<Asset>>;

	class EditorAssetManager : public AssetManagerBase
	{
	public:
		EditorAssetManager();
		~EditorAssetManager() override;

		void LoadBuiltInAssets();

		AssetType GetAssetType(AssetHandle aHandle) override;
		std::shared_ptr<Asset> GetAsset(AssetHandle aHandle) override;
		std::shared_ptr<Asset> GetAssetAsync(AssetHandle aHandle) override;
		void AddMemoryOnlyAsset(std::shared_ptr<Asset> aAsset) override;
		void AddMemoryOnlyAsset(std::shared_ptr<Asset> aAsset, const std::string& aName) override;

		bool ReloadData(AssetHandle aAssetHandle) override;
		void RemoveAsset(AssetHandle aHandle) override;

		bool IsMemoryAsset(AssetHandle handle) override;
		bool IsAssetHandleValid(AssetHandle aHandle) override;
		bool IsAssetLoaded(AssetHandle aHandle) override;
		bool IsAssetValid(AssetHandle aHandle) override;
		bool IsAssetMissing(AssetHandle aHandle) override;

		std::unordered_set<AssetHandle> GetAllAssetsWithType(AssetType aType) override;
		const std::unordered_map<AssetHandle, std::shared_ptr<Asset>>& GetLoadedAssets() override;

		const AssetMetadata& GetMetadata(AssetHandle aHandle);
		AssetMetadata& GetMutableMetadata(AssetHandle aHandle);
		const AssetMetadata& GetMetadata(const std::filesystem::path& aFilepath);
		const AssetMetadata& GetMetadata(const std::shared_ptr<Asset>& aAsset);
		
		AssetHandle ImportAsset(const std::filesystem::path& aFilepath);
		
		AssetHandle GetAssetHandleFromFilePath(const std::filesystem::path& aFilepath);

		AssetType GetAssetTypeFromExtension(const std::string& aExtension);
		AssetType GetAssetTypeFromPath(const std::filesystem::path& aPath);

		std::filesystem::path GetFileSystemPath(AssetHandle aHandle);
		std::filesystem::path GetFileSystemPath(const AssetMetadata& aMetadata);
		std::filesystem::path GetRelativePath(const std::filesystem::path& aFilepath);

		bool FileExists(AssetMetadata& aMetadata) const;

		const AssetRegistry& GetAssetRegistry() const { return myAssetRegistry; }
		
		void OnAssetRenamed(AssetHandle aAssetHandle, const std::filesystem::path& aNewFilePath);
		void OnAssetDeleted(AssetHandle aAssetHandle);

		template<typename T, typename... Args>
		std::shared_ptr<T> CreateNewAsset(const std::string& aFilename, const std::string& aDirectoryPath, Args&&... aArgs)
		{
			static_assert(std::is_base_of<Asset, T>::value, "CreateNewAsset only works for types derived from Asset");

			AssetMetadata metadata;
			metadata.handle = AssetHandle();
			if (aDirectoryPath.empty() || aDirectoryPath == ".")
			{
				metadata.filePath = aFilename;
			}
			else
			{
				metadata.filePath = GetRelativePath(aDirectoryPath + "/" + aFilename);
			}
			metadata.isDataLoaded = true;
			metadata.type = T::GetStaticType();

			myAssetRegistry[metadata.handle] = metadata;

			std::shared_ptr<T> asset = std::make_shared<T>(std::forward<Args>(aArgs)...);
			asset->myHandle = metadata.handle;
			myLoadedAssets[asset->myHandle] = asset;

			AssetImporter::Serialize(metadata, asset);;

			SerializeAssetRegistry();

			return asset;
		}

	private:
		void SerializeAssetRegistry();
		void DeserializeAssetRegistry();

		void ProcessDirectory(const std::filesystem::path& aDirectoryPath);
		void ReloadAssets();

		std::shared_ptr<Asset> GetAssetIncludingInvalid(AssetHandle assetHandle);
		AssetMetadata& GetMetadataInternal(AssetHandle aHandle);

	private:
		AssetMap myLoadedAssets;
		AssetMap myMemoryAssets;
		AssetRegistry myAssetRegistry;

		std::unordered_map<AssetHandle, std::future<std::shared_ptr<Asset>>> myLoadingAssets;
	};
}
