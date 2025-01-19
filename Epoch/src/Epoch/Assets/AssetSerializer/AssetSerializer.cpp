#include "epch.h"
#include "AssetSerializer.h"
#include "Epoch/Scene/Prefab.h"
#include "Epoch/Scene/SceneSerializer.h"
#include "Epoch/Rendering/Font.h"
#include "Epoch/Rendering/Texture.h"
#include "Epoch/Rendering/Renderer.h"
#include "Epoch/Rendering/Environment.h"
#include "Epoch/Rendering/Material.h"
#include "Epoch/Script/ScriptAsset.h"
#include "Epoch/Project/Project.h"
#include "Epoch/Assets/AssetManager.h"
#include "Epoch/Assets/AssimpMeshImporter.h"
#include "Epoch/Assets/AssetSerializer/Runtime/TextureRuntimeSerializer.h"

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

	bool SceneAssetSerializer::SerializeToAssetPack(AssetHandle aHandle, FileStreamWriter& aStream, AssetSerializationInfo& outInfo) const
	{
		std::shared_ptr<Scene> scene = std::make_shared<Scene>(aHandle);
		const auto& metadata = Project::GetEditorAssetManager()->GetMetadata(aHandle);
		SceneSerializer serializer(scene);
		if (serializer.Deserialize(Project::GetAssetDirectory() / metadata.filePath))
		{
			return serializer.SerializeToAssetPack(aStream, outInfo);
		}
		return false;
	}

	std::shared_ptr<Asset> SceneAssetSerializer::DeserializeFromAssetPack(FileStreamReader& aStream, const AssetPackFile::AssetInfo& aAssetInfo) const
	{
		EPOCH_ASSERT("Not implemented! Use 'DeserializeSceneFromAssetPack'.");
		return nullptr;
	}

	std::shared_ptr<Scene> SceneAssetSerializer::DeserializeSceneFromAssetPack(FileStreamReader& aStream, const AssetPackFile::SceneInfo& aSceneInfo) const
	{
		std::shared_ptr<Scene> scene = std::make_shared<Scene>();
		SceneSerializer serializer(scene);
		if (serializer.DeserializeFromAssetPack(aStream, aSceneInfo))
		{
			return scene;
		}

		return nullptr;
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

	bool PrefabSerializer::SerializeToAssetPack(AssetHandle aHandle, FileStreamWriter& aStream, AssetSerializationInfo& outInfo) const
	{
		return false;
	}

	std::shared_ptr<Asset> PrefabSerializer::DeserializeFromAssetPack(FileStreamReader& aStream, const AssetPackFile::AssetInfo& aAssetInfo) const
	{
		return std::shared_ptr<Asset>();
	}

	std::string PrefabSerializer::SerializeToYAML(std::shared_ptr<Prefab> aPrefab) const
	{
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Prefab";
		out << YAML::Value << YAML::BeginSeq;

		SceneSerializer sceneSerializer(aPrefab->myScene);
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

	bool TextureSerializer::SerializeToAssetPack(AssetHandle aHandle, FileStreamWriter& aStream, AssetSerializationInfo& outInfo) const
	{
		outInfo.offset = aStream.GetStreamPosition();

		auto& metadata = Project::GetEditorAssetManager()->GetMetadata(aHandle);
		std::shared_ptr<Texture2D> texture = AssetManager::GetAsset<Texture2D>(aHandle);
		outInfo.size = TextureRuntimeSerializer::SerializeTexture2DToFile(texture, aStream);
		return outInfo.size > 0;
	}

	std::shared_ptr<Asset> TextureSerializer::DeserializeFromAssetPack(FileStreamReader& aStream, const AssetPackFile::AssetInfo& aAssetInfo) const
	{
		aStream.SetStreamPosition(aAssetInfo.packedOffset);
		return TextureRuntimeSerializer::DeserializeTexture2D(aStream);
	}
	

	bool FontSerializer::TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const
	{
		aAsset = std::make_shared<Font>(Project::GetEditorAssetManager()->GetFileSystemPath(aMetadata.handle));
		aAsset->myHandle = aMetadata.handle;
		return true;
	}

	bool FontSerializer::SerializeToAssetPack(AssetHandle aHandle, FileStreamWriter& aStream, AssetSerializationInfo& outInfo) const
	{
		outInfo.offset = aStream.GetStreamPosition();

		std::shared_ptr<Font> font = AssetManager::GetAsset<Font>(aHandle);
		auto path = Project::GetEditorAssetManager()->GetFileSystemPath(aHandle);
		aStream.WriteString(font->GetName());
		Buffer fontData = FileSystem::ReadBytes(path);
		aStream.WriteBuffer(fontData);

		outInfo.size = aStream.GetStreamPosition() - outInfo.offset;
		return outInfo.size > 0;
	}

	std::shared_ptr<Asset> FontSerializer::DeserializeFromAssetPack(FileStreamReader& aStream, const AssetPackFile::AssetInfo& aAssetInfo) const
	{
		aStream.SetStreamPosition(aAssetInfo.packedOffset);

		std::string name;
		aStream.ReadString(name);
		Buffer fontData;
		aStream.ReadBuffer(fontData);

		return std::make_shared<Font>(name, fontData);
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

	bool EnvironmentSerializer::SerializeToAssetPack(AssetHandle aHandle, FileStreamWriter& aStream, AssetSerializationInfo& outInfo) const
	{
		return false;
	}

	std::shared_ptr<Asset> EnvironmentSerializer::DeserializeFromAssetPack(FileStreamReader& aStream, const AssetPackFile::AssetInfo& aAssetInfo) const
	{
		return std::shared_ptr<Asset>();
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

	bool MeshSerializer::SerializeToAssetPack(AssetHandle aHandle, FileStreamWriter& aStream, AssetSerializationInfo& outInfo) const
	{
		return false;
	}

	std::shared_ptr<Asset> MeshSerializer::DeserializeFromAssetPack(FileStreamReader& aStream, const AssetPackFile::AssetInfo& aAssetInfo) const
	{
		return std::shared_ptr<Asset>();
	}


	void MaterialSerializer::Serialize(const AssetMetadata& aMetadata, const std::shared_ptr<Asset>& aAsset) const
	{
		std::shared_ptr<Material> material = std::static_pointer_cast<Material>(aAsset);

		std::string yamlString = SerializeToYAML(material);

		std::ofstream fout(Project::GetEditorAssetManager()->GetFileSystemPath(aMetadata));
		fout << yamlString;
		fout.close();
	}

	bool MaterialSerializer::TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& aAsset) const
	{
		aAsset = std::make_shared<Material>();
		aAsset->myHandle = aMetadata.handle;

		std::shared_ptr<Material> material = std::static_pointer_cast<Material>(aAsset);

		std::ifstream stream(Project::GetEditorAssetManager()->GetFileSystemPath(aMetadata));
		if (!stream.is_open())
		{
			aAsset->SetFlag(AssetFlag::Invalid);
			return false;
		}

		std::stringstream strStream;
		strStream << stream.rdbuf();

		bool success = DeserializeFromYAML(strStream.str(), material);
		if (!success)
		{
			aAsset->SetFlag(AssetFlag::Invalid);
			return false;
		}
		
		return true;
	}

	bool MaterialSerializer::SerializeToAssetPack(AssetHandle aHandle, FileStreamWriter& aStream, AssetSerializationInfo& outInfo) const
	{
		std::shared_ptr<Material> materialAsset = AssetManager::GetAsset<Material>(aHandle);

		std::string yamlString = SerializeToYAML(materialAsset);
		outInfo.offset = aStream.GetStreamPosition();
		aStream.WriteString(yamlString);
		outInfo.size = aStream.GetStreamPosition() - outInfo.offset;
		return outInfo.size > 0;
	}

	std::shared_ptr<Asset> MaterialSerializer::DeserializeFromAssetPack(FileStreamReader& aStream, const AssetPackFile::AssetInfo& aAssetInfo) const
	{
		aStream.SetStreamPosition(aAssetInfo.packedOffset);
		std::string yamlString;
		aStream.ReadString(yamlString);

		std::shared_ptr<Material> materialAsset;
		bool result = DeserializeFromYAML(yamlString, materialAsset);
		if (!result)
		{
			return nullptr;
		}

		return materialAsset;
	}

	std::string MaterialSerializer::SerializeToYAML(std::shared_ptr<Material> aMaterial) const
	{
		YAML::Emitter out;

		out << YAML::BeginMap;

		out << YAML::Key << "AlbedoColor" << YAML::Value << aMaterial->GetAlbedoColor();
		out << YAML::Key << "Roughness" << YAML::Value << aMaterial->GetRoughness();
		out << YAML::Key << "Metalness" << YAML::Value << aMaterial->GetMetalness();
		out << YAML::Key << "NormalStrength" << YAML::Value << aMaterial->GetNormalStrength();
		out << YAML::Key << "UVTiling" << YAML::Value << aMaterial->GetUVTiling();
		out << YAML::Key << "EmissionColor" << YAML::Value << aMaterial->GetEmissionColor();
		out << YAML::Key << "EmissionStrength" << YAML::Value << aMaterial->GetEmissionStrength();

		out << YAML::Key << "AlbedoTexture" << YAML::Value << aMaterial->myAlbedoTexture;
		out << YAML::Key << "NormalTexture" << YAML::Value << aMaterial->myNormalTexture;
		out << YAML::Key << "MaterialTexture" << YAML::Value << aMaterial->myMaterialTexture;

		out << YAML::EndMap;

		return std::string(out.c_str());
	}

	bool MaterialSerializer::DeserializeFromYAML(const std::string& yamlString, std::shared_ptr<Material>& aMaterial) const
	{
		YAML::Node data = YAML::Load(yamlString);

		aMaterial->SetAlbedoColor(data["AlbedoColor"].as<CU::Vector3f>(CU::Vector3f::One));
		aMaterial->SetRoughness(data["Roughness"].as<float>(1.0f));
		aMaterial->SetMetalness(data["Metalness"].as<float>(0.0f));
		aMaterial->SetNormalStrength(data["NormalStrength"].as<float>(1.0f));
		aMaterial->SetUVTiling(data["UVTiling"].as<CU::Vector2f>(CU::Vector2f::One));
		aMaterial->SetEmissionColor(data["EmissionColor"].as<CU::Vector3f>(CU::Vector3f::One));
		aMaterial->SetEmissionStrength(data["EmissionStrength"].as<float>(0.0f));

		aMaterial->SetAlbedoTexture(data["AlbedoTexture"].as<UUID>(UUID(0)));
		aMaterial->SetNormalTexture(data["NormalTexture"].as<UUID>(UUID(0)));
		aMaterial->SetMaterialTexture(data["MaterialTexture"].as<UUID>(UUID(0)));

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

	bool ScriptFileSerializer::SerializeToAssetPack(AssetHandle aHandle, FileStreamWriter& aStream, AssetSerializationInfo& outInfo) const
	{
		EPOCH_ASSERT("Not implemented!");
		return false;
	}
	
	std::shared_ptr<Asset> ScriptFileSerializer::DeserializeFromAssetPack(FileStreamReader& aStream, const AssetPackFile::AssetInfo& aAssetInfo) const
	{
		EPOCH_ASSERT("Not implemented!");
		return nullptr;
	}
}
