#pragma once
#include <string>
#include <memory>
#include "Epoch/Serialization/FileStream.h"
#include "Epoch/Assets/AssetMetadata.h"
#include "Epoch/Assets/AssetPack/AssetPackFile.h"
#include "Epoch/Utils/YAMLSerializationHelpers.h"

namespace Epoch
{
	class Prefab;
	class Scene;

	struct AssetSerializationInfo
	{
		uint64_t offset = 0;
		uint64_t size = 0;
	};

	class AssetSerializer
	{
	public:
		virtual void Serialize(const AssetMetadata& aMetadata, const std::shared_ptr<Asset>& aAsset) const = 0;
		virtual bool TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const = 0;

		virtual bool SerializeToAssetPack(AssetHandle aHandle, FileStreamWriter& aStream, AssetSerializationInfo& outInfo) const = 0;
		virtual std::shared_ptr<Asset> DeserializeFromAssetPack(FileStreamReader& aStream, const AssetPackFile::AssetInfo& aAssetInfo) const = 0;
	};

	class SceneAssetSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& aMetadata, const std::shared_ptr<Asset>& aAsset) const override;
		bool TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const override;

		bool SerializeToAssetPack(AssetHandle aHandle, FileStreamWriter& aStream, AssetSerializationInfo& outInfo) const override;
		std::shared_ptr<Asset> DeserializeFromAssetPack(FileStreamReader& aStream, const AssetPackFile::AssetInfo& aAssetInfo) const override;
		std::shared_ptr<Scene> DeserializeSceneFromAssetPack(FileStreamReader& aStream, const AssetPackFile::SceneInfo& aSceneInfo) const;
	};

	class PrefabSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& aMetadata, const std::shared_ptr<Asset>& aAsset) const override;
		bool TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const override;

		bool SerializeToAssetPack(AssetHandle aHandle, FileStreamWriter& aStream, AssetSerializationInfo& outInfo) const override;
		std::shared_ptr<Asset> DeserializeFromAssetPack(FileStreamReader& aStream, const AssetPackFile::AssetInfo& aAssetInfo) const override;

	private:
		std::string SerializeToYAML(std::shared_ptr<Prefab> aPrefab) const;
		bool DeserializeFromYAML(YAML::Node& aData, std::shared_ptr<Prefab> aPrefab) const;
	};

	class TextureSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& aMetadata, const std::shared_ptr<Asset>& aAsset) const override {}
		bool TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const override;

		bool SerializeToAssetPack(AssetHandle aHandle, FileStreamWriter& aStream, AssetSerializationInfo& outInfo) const override;
		std::shared_ptr<Asset> DeserializeFromAssetPack(FileStreamReader& aStream, const AssetPackFile::AssetInfo& aAssetInfo) const override;
	};

	class FontSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& aMetadata, const std::shared_ptr<Asset>& aAsset) const override {}
		bool TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const override;

		bool SerializeToAssetPack(AssetHandle aHandle, FileStreamWriter& aStream, AssetSerializationInfo& outInfo) const override;
		std::shared_ptr<Asset> DeserializeFromAssetPack(FileStreamReader& aStream, const AssetPackFile::AssetInfo& aAssetInfo) const override;
	};

	class EnvironmentSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& aMetadata, const std::shared_ptr<Asset>& aAsset) const override {}
		bool TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const override;

		bool SerializeToAssetPack(AssetHandle aHandle, FileStreamWriter& aStream, AssetSerializationInfo& outInfo) const override;
		std::shared_ptr<Asset> DeserializeFromAssetPack(FileStreamReader& aStream, const AssetPackFile::AssetInfo& aAssetInfo) const override;
	};

	class MeshSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& aMetadata, const std::shared_ptr<Asset>& aAsset) const override {}
		bool TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const override;

		bool SerializeToAssetPack(AssetHandle aHandle, FileStreamWriter& aStream, AssetSerializationInfo& outInfo) const override;
		std::shared_ptr<Asset> DeserializeFromAssetPack(FileStreamReader& aStream, const AssetPackFile::AssetInfo& aAssetInfo) const override;
	};

	class MaterialSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& aMetadata, const std::shared_ptr<Asset>& aAsset) const override;
		bool TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const override;

		bool SerializeToAssetPack(AssetHandle aHandle, FileStreamWriter& aStream, AssetSerializationInfo& outInfo) const override;
		std::shared_ptr<Asset> DeserializeFromAssetPack(FileStreamReader& aStream, const AssetPackFile::AssetInfo& aAssetInfo) const override;

	private:
		std::string SerializeToYAML(std::shared_ptr<Material> aMaterial) const;
		bool DeserializeFromYAML(const std::string& yamlString, std::shared_ptr<Material>& aMaterial) const;
	};

	class ScriptFileSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& aMetadata, const std::shared_ptr<Asset>& aAsset) const override;
		bool TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const override;

		bool SerializeToAssetPack(AssetHandle aHandle, FileStreamWriter& aStream, AssetSerializationInfo& outInfo) const override;
		std::shared_ptr<Asset> DeserializeFromAssetPack(FileStreamReader& aStream, const AssetPackFile::AssetInfo& aAssetInfo) const override;
	};
}
