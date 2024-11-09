#include "epch.h"
#include "AssetImporter.h"
#include "Epoch/Assets/AssetExtensions.h"
#include "Epoch/Project/Project.h"

namespace Epoch
{
	void Epoch::AssetImporter::Init()
	{
		staticSerializers.clear();
		staticSerializers[AssetType::Scene]			= std::make_shared<SceneAssetSerializer>();
		staticSerializers[AssetType::Prefab]		= std::make_shared<PrefabSerializer>();
		staticSerializers[AssetType::Texture]		= std::make_shared<TextureSerializer>();
		staticSerializers[AssetType::Font]			= std::make_shared<FontSerializer>();
		staticSerializers[AssetType::EnvTexture]	= std::make_shared<EnvironmentSerializer>();
		staticSerializers[AssetType::Mesh]			= std::make_shared<MeshSerializer>();
		staticSerializers[AssetType::Material]		= std::make_shared<MaterialSerializer>();
		staticSerializers[AssetType::ScriptFile]	= std::make_shared<ScriptFileSerializer>();
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
}
