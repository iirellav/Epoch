#pragma once
#include "Epoch/Assets/AssetSerializer/AssetSerializer.h"
#include "Epoch/Serialization/FileStream.h"

namespace Epoch
{
	class Asset;
	class Scene;

	class AssetImporter
	{
	public:
		static void Init();
		static void Serialize(const AssetMetadata& aMetadata, const std::shared_ptr<Asset>& aAsset);
		static void Serialize(const std::shared_ptr<Asset>& aAsset);
		static bool TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset);

		static bool SerializeToAssetPack(AssetHandle aHandle, FileStreamWriter& aStream, AssetSerializationInfo& outInfo);
		static std::shared_ptr<Asset> DeserializeFromAssetPack(FileStreamReader& aStream, const AssetPackFile::AssetInfo& aAssetInfo);
		static std::shared_ptr<Scene> DeserializeSceneFromAssetPack(FileStreamReader& aStream, const AssetPackFile::SceneInfo& aSceneInfo);

	private:
		inline static std::unordered_map<AssetType, std::unique_ptr<AssetSerializer>> staticSerializers;
	};
}
