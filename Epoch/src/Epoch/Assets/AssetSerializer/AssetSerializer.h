#pragma once
#include <memory>
#include "Epoch/Assets/AssetMetadata.h"
#include "Epoch/Utils/YAMLSerializationHelpers.h"

namespace Epoch
{
	class Prefab;

	class AssetSerializer
	{
	public:
		virtual void Serialize(const AssetMetadata& aMetadata, const std::shared_ptr<Asset>& aAsset) const = 0;
		virtual bool TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const = 0;
	};

	class SceneAssetSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& aMetadata, const std::shared_ptr<Asset>& aAsset) const override;
		bool TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const override;
	};

	class PrefabSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& aMetadata, const std::shared_ptr<Asset>& aAsset) const override;
		bool TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const override;

	private:
		std::string SerializeToYAML(std::shared_ptr<Prefab> prefab) const;
		bool DeserializeFromYAML(YAML::Node& aData, std::shared_ptr<Prefab> aPrefab) const;
	};

	class TextureSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& aMetadata, const std::shared_ptr<Asset>& aAsset) const override {}
		bool TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const override;
	};

	class FontSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& aMetadata, const std::shared_ptr<Asset>& aAsset) const override {}
		bool TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const override;
	};

	class EnvironmentSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& aMetadata, const std::shared_ptr<Asset>& aAsset) const override {}
		bool TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const override;
	};

	class ModelSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& aMetadata, const std::shared_ptr<Asset>& aAsset) const override {}
		bool TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const override;
	};

	class MaterialSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& aMetadata, const std::shared_ptr<Asset>& aAsset) const override;
		bool TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const override;
	};

	class ScriptFileSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& aMetadata, const std::shared_ptr<Asset>& aAsset) const override;
		bool TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const override;
	};
}
