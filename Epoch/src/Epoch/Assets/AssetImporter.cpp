#include "epch.h"
#include "AssetImporter.h"
#include "Epoch/Assets/AssetExtensions.h"
#include "Epoch/Assets/AssetManager.h"
#include "Epoch/Project/Project.h"

namespace Epoch
{
	void Epoch::AssetImporter::Init()
	{
		staticSerializers.clear();
		staticSerializers[AssetType::Scene]			= std::make_unique<SceneAssetSerializer>();
		staticSerializers[AssetType::Prefab]		= std::make_unique<PrefabSerializer>();
		staticSerializers[AssetType::Texture]		= std::make_unique<TextureSerializer>();
		staticSerializers[AssetType::Font]			= std::make_unique<FontSerializer>();
		staticSerializers[AssetType::EnvTexture]	= std::make_unique<EnvironmentSerializer>();
		staticSerializers[AssetType::Mesh]			= std::make_unique<MeshSerializer>();
		staticSerializers[AssetType::Material]		= std::make_unique<MaterialSerializer>();
		staticSerializers[AssetType::ScriptFile]	= std::make_unique<ScriptFileSerializer>();
	}

	void Epoch::AssetImporter::Serialize(const AssetMetadata& aMetadata, const std::shared_ptr<Asset>& aAsset)
	{
		if (staticSerializers.find(aMetadata.type) == staticSerializers.end())
		{
			LOG_WARNING("There's currently no importer for assets of type '{}'", AssetTypeToString(aMetadata.type));
			return;
		}

		staticSerializers[aMetadata.type]->Serialize(aMetadata, aAsset);
	}

	void Epoch::AssetImporter::Serialize(const std::shared_ptr<Asset>& aAsset)
	{
		const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(aAsset->GetHandle());
		Serialize(metadata, aAsset);
	}

	bool Epoch::AssetImporter::TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset)
	{
		if (staticSerializers.find(aMetadata.type) == staticSerializers.end())
		{
			LOG_WARNING("There's currently no importer for assets of type '{}'", AssetTypeToString(aMetadata.type));
			return false;
		}

		return staticSerializers[aMetadata.type]->TryLoadData(aMetadata, aAsset);
	}

	bool AssetImporter::SerializeToAssetPack(AssetHandle aHandle, FileStreamWriter& aStream, AssetSerializationInfo& outInfo)
	{
		if (!AssetManager::IsAssetHandleValid(aHandle))
		{
			return false;
		}

		const auto& metadata = Project::GetEditorAssetManager()->GetMetadata(aHandle);
		if (staticSerializers.find(metadata.type) == staticSerializers.end())
		{
			LOG_WARNING("There's currently no importer for assets of type {}", AssetTypeToString(metadata.type));
			return false;
		}

		return staticSerializers[metadata.type]->SerializeToAssetPack(aHandle, aStream, outInfo);
	}
}
