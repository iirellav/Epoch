#include "epch.h"
#include "AssetSerializer.h"
#include "Epoch/Scene/Prefab.h"
#include "Epoch/Rendering/Font.h"
#include "Epoch/Rendering/Texture.h"
#include "Epoch/Rendering/Renderer.h"
#include "Epoch/Rendering/Environment.h"
#include "Epoch/Rendering/Material.h"
#include "Epoch/Script/ScriptAsset.h"
#include "Epoch/Project/Project.h"
#include "Epoch/Assets/AssimpMeshImporter.h"
#include "Epoch/Scene/SceneSerializer.h"

namespace Epoch
{
	void SceneAssetSerializer::Serialize(const AssetMetadata& aMetadata, const std::shared_ptr<Asset>& aAsset) const
	{
		std::shared_ptr<Scene> scene = std::static_pointer_cast<Scene>(aAsset);
		SceneSerializer serializer(scene);
		serializer.Serialize(Project::GetEditorAssetManager()->GetFileSystemPath(aMetadata));
	}

	bool SceneAssetSerializer::TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const
	{
		aAsset = std::make_shared<Scene>("SceneAsset");
		aAsset->myHandle = aMetadata.handle;
		return true;
	}

	void PrefabSerializer::Serialize(const AssetMetadata& aMetadata, const std::shared_ptr<Asset>& aAsset) const
	{
		std::shared_ptr<Prefab> prefab = std::static_pointer_cast<Prefab>(aAsset);
		
		std::string yamlString = SerializeToYAML(prefab);

		std::ofstream fout(Project::GetEditorAssetManager()->GetFileSystemPath(aMetadata));
		fout << yamlString;
	}

	bool PrefabSerializer::TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(Project::GetEditorAssetManager()->GetFileSystemPath(aMetadata).string());
		}
		catch (YAML::ParserException e)
		{
			LOG_ERROR_TAG("AssetManager", "Failed to deserialize prefab '{}': {}", Project::GetEditorAssetManager()->GetFileSystemPath(aMetadata).string(), e.what());
			return false;
		}
		
		aAsset = std::make_shared<Prefab>();
		aAsset->myHandle = aMetadata.handle;
		std::shared_ptr<Prefab> prefabAsset = std::static_pointer_cast<Prefab>(aAsset);
		bool success = DeserializeFromYAML(data, prefabAsset);
		if (!success)
		{
			aAsset->SetFlag(AssetFlag::Invalid);
			return false;
		}

		auto entities = prefabAsset->myScene->GetAllEntitiesWith<RelationshipComponent>();
		for (auto e : entities)
		{
			Entity entity = { e, prefabAsset->myScene.get()};
			if (!entity.GetParent())
			{
				prefabAsset->myEntity = entity;
				break;
			}
		}

		return true;
	}

	std::string PrefabSerializer::SerializeToYAML(std::shared_ptr<Prefab> prefab) const
	{
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Prefab";
		out << YAML::Value << YAML::BeginSeq;

		SceneSerializer sceneSerializer(prefab->myScene);
		sceneSerializer.SerializeEntities(out);

		out << YAML::EndSeq;
		out << YAML::EndMap;

		return std::string(out.c_str());
	}

	bool PrefabSerializer::DeserializeFromYAML(YAML::Node& aData, std::shared_ptr<Prefab> aPrefab) const
	{
		if (!aData["Prefab"])
		{
			return false;
		}

		YAML::Node prefabNode = aData["Prefab"];
		SceneSerializer sceneSerializer(aPrefab->myScene);
		sceneSerializer.DeserializeEntities(prefabNode);

		return true;
	}

	bool TextureSerializer::TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const
	{
		aAsset = Texture2D::Create(Project::GetEditorAssetManager()->GetFileSystemPath(aMetadata));
		aAsset->myHandle = aMetadata.handle;

		const bool result = std::static_pointer_cast<Texture2D>(aAsset)->Loaded();
		if (!result)
		{
			aAsset->SetFlag(AssetFlag::Invalid);
			return false;
		}

		return result;
	}
	
	bool FontSerializer::TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const
	{
		aAsset = std::make_shared<Font>(Project::GetEditorAssetManager()->GetFileSystemPath(aMetadata.handle));
		aAsset->myHandle = aMetadata.handle;
		return true;
	}

	bool EnvironmentSerializer::TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const
	{
		auto [radiance, irradiance] = Renderer::CreateEnvironmentTextures(Project::GetEditorAssetManager()->GetFileSystemPath(aMetadata).string());
		
		aAsset = std::make_shared<Environment>();
		aAsset->myHandle = aMetadata.handle;
		if (!radiance/* || !irradiance*/)
		{
			aAsset->SetFlag(AssetFlag::Invalid);
			return false;
		}
		
		aAsset = std::make_shared<Environment>(radiance, irradiance);
		aAsset->myHandle = aMetadata.handle;
		return true;
	}
	
	bool MeshSerializer::TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const
	{
		EPOCH_PROFILE_FUNC();

		AssimpMeshImporter importer(Project::GetEditorAssetManager()->GetFileSystemPath(aMetadata).string());
		aAsset = importer.ImportMesh();
		aAsset->myHandle = aMetadata.handle;
		
		auto mesh = std::static_pointer_cast<Mesh>(aAsset);
		const bool result = mesh->GetVertexBuffer() && mesh->GetIndexBuffer();
		if (!result)
		{
			aAsset->SetFlag(AssetFlag::Invalid);
			return false;
		}

		return true;
	}

	void MaterialSerializer::Serialize(const AssetMetadata& aMetadata, const std::shared_ptr<Asset>& aAsset) const
	{
		std::shared_ptr<Material> material = std::static_pointer_cast<Material>(aAsset);

		YAML::Emitter out;
		
		out << YAML::BeginMap;
		
		out << YAML::Key << "AlbedoColor" << YAML::Value << material->GetAlbedoColor();
		out << YAML::Key << "Roughness" << YAML::Value << material->GetRoughness();
		out << YAML::Key << "Metalness" << YAML::Value << material->GetMetalness();
		out << YAML::Key << "NormalStrength" << YAML::Value << material->GetNormalStrength();
		out << YAML::Key << "UVTiling" << YAML::Value << material->GetUVTiling();
		out << YAML::Key << "EmissionColor" << YAML::Value << material->GetEmissionColor();
		out << YAML::Key << "EmissionStrength" << YAML::Value << material->GetEmissionStrength();

		out << YAML::Key << "AlbedoTexture" << YAML::Value << material->myAlbedoTexture;
		out << YAML::Key << "NormalTexture" << YAML::Value << material->myNormalTexture;
		out << YAML::Key << "MaterialTexture" << YAML::Value << material->myMaterialTexture;
		
		out << YAML::EndMap;

		std::ofstream fout(Project::GetEditorAssetManager()->GetFileSystemPath(aMetadata));
		fout << out.c_str();
		fout.close();
	}

	bool MaterialSerializer::TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const
	{
		aAsset = std::make_shared<Material>();
		aAsset->myHandle = aMetadata.handle;

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(Project::GetEditorAssetManager()->GetFileSystemPath(aMetadata).string());
		}
		catch (YAML::ParserException e)
		{
			aAsset->SetFlag(AssetFlag::Invalid);
			LOG_ERROR("Failed to deserialize material '{}': {}", aMetadata.filePath.string(), e.what());
			return false;
		}
		
		std::shared_ptr<Material> material = std::static_pointer_cast<Material>(aAsset);

		material->SetAlbedoColor(data["AlbedoColor"].as<CU::Vector3f>(CU::Vector3f::One));
		material->SetRoughness(data["Roughness"].as<float>(1.0f));
		material->SetMetalness(data["Metalness"].as<float>(0.0f));
		material->SetNormalStrength(data["NormalStrength"].as<float>(1.0f));
		material->SetUVTiling(data["UVTiling"].as<CU::Vector2f>(CU::Vector2f::One));
		material->SetEmissionColor(data["EmissionColor"].as<CU::Vector3f>(CU::Vector3f::One));
		material->SetEmissionStrength(data["EmissionStrength"].as<float>(0.0f));

		material->SetAlbedoTexture(data["AlbedoTexture"].as<UUID>(UUID(0)));
		material->SetNormalTexture(data["NormalTexture"].as<UUID>(UUID(0)));
		material->SetMaterialTexture(data["MaterialTexture"].as<UUID>(UUID(0)));

		return true;
	}

	void ScriptFileSerializer::Serialize(const AssetMetadata& aMetadata, const std::shared_ptr<Asset>& aAsset) const
	{
		std::ofstream stream(Project::GetEditorAssetManager()->GetFileSystemPath(aMetadata));
		EPOCH_ASSERT(stream.is_open(), "Failed to open stream!");

		std::ifstream templateStream("Resources/Templates/NewClassTemplate.cs");
		EPOCH_ASSERT(templateStream.is_open(), "Failed to open template!");

		std::stringstream templateStrStream;
		templateStrStream << templateStream.rdbuf();
		std::string templateString = templateStrStream.str();

		templateStream.close();

		auto replaceTemplateToken = [&templateString](const char* token, const std::string& value)
		{
			size_t pos = 0;
			while ((pos = templateString.find(token, pos)) != std::string::npos)
			{
				templateString.replace(pos, strlen(token), value);
				pos += strlen(token);
			}
		};

		auto scriptFileAsset = std::dynamic_pointer_cast<ScriptFileAsset>(aAsset);
		replaceTemplateToken("$NAMESPACE_NAME$", scriptFileAsset->GetScriptNamespace());
		replaceTemplateToken("$CLASS_NAME$", scriptFileAsset->GetScriptName());

		stream << templateString;
		stream.close();
	}

	bool ScriptFileSerializer::TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const
	{
		aAsset = std::make_shared<ScriptFileAsset>();
		aAsset->myHandle = aMetadata.handle;
		return true;
	}
}
