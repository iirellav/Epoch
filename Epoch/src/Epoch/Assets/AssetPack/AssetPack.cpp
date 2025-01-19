#include "epch.h"
#include <unordered_set>
#include "AssetPack.h"
#include "AssetPackSerializer.h"
#include "Epoch/Core/Platform.h"
#include "Epoch/Project/Project.h"
#include "Epoch/Assets/AssetManager.h"
#include "Epoch/Scene/SceneSerializer.h"

namespace Epoch
{
	static void GetAllReferencedScenes(AssetHandle aCurrentScene, std::unordered_set<AssetHandle>& outReferencedScenes)
	{
		std::unordered_set<AssetHandle> referencedScenes;

		//Load current scene and store all referenced scenes in a set
		{
			const AssetRegistry& registry = Project::GetEditorAssetManager()->GetAssetRegistry();

			const auto& metadata = registry.Get(aCurrentScene);
			std::shared_ptr<Scene> scene = std::make_shared<Scene>(aCurrentScene);
			SceneSerializer serializer(scene);
			if (serializer.Deserialize(Project::GetAssetDirectory() / metadata.filePath))
			{
				//Only add the scene handle to 'referencedScenes' if it's not already in 'outReferencedScenes'
				std::unordered_set<AssetHandle> sceneReferences = scene->GetAllSceneReferences();
				for (AssetHandle sceneHandle : sceneReferences)
				{
					if (outReferencedScenes.find(sceneHandle) == outReferencedScenes.end())
					{
						referencedScenes.insert(sceneHandle);
					}
				}
			}
		}

		outReferencedScenes.insert(referencedScenes.begin(), referencedScenes.end());

		//Call this function for each scene in the set
		for (AssetHandle sceneHandle : referencedScenes)
		{
			GetAllReferencedScenes(sceneHandle, outReferencedScenes);
		}
	}

	bool AssetPack::CreateFromActiveProject(std::filesystem::path aDestination)
	{
		AssetPackFile assetPackFile;
		assetPackFile.header.buildVersion = Platform::GetCurrentDateTimeU64();

		AssetHandle startSceneHandle = Project::GetActive()->GetConfig().startScene;
		const AssetRegistry& registry = Project::GetEditorAssetManager()->GetAssetRegistry();

		if (startSceneHandle == 0 || !registry.Contains(startSceneHandle))
		{
			myLastBuildError = BuildError::NoStartScene;
			return false;
		}

		std::unordered_set<AssetHandle> referencedScenes;
		referencedScenes.insert(startSceneHandle);
		GetAllReferencedScenes(startSceneHandle, referencedScenes);

		std::unordered_set<AssetHandle> fullAssetList;
		for (AssetHandle sceneHandle : referencedScenes)
		{
			const auto& metadata = registry.Get(sceneHandle);
			std::shared_ptr<Scene> scene = std::make_shared<Scene>(sceneHandle);
			SceneSerializer serializer(scene);
			LOG_DEBUG("Deserializing Scene: {}", metadata.filePath);
			if (serializer.Deserialize(Project::GetAssetDirectory() / metadata.filePath))
			{
				std::unordered_set<AssetHandle> sceneAssetList = scene->GetAllSceneAssets();
				LOG_INFO("  Scene has {} used assets", sceneAssetList.size());

				std::unordered_set<AssetHandle> sceneAssetListWithoutPrefabs = sceneAssetList;
				for (AssetHandle assetHandle : sceneAssetListWithoutPrefabs)
				{
					const auto& metadata = Project::GetEditorAssetManager()->GetMetadata(assetHandle);
					if (metadata.type == AssetType::Prefab)
					{
						std::shared_ptr<Prefab> prefab = AssetManager::GetAsset<Prefab>(assetHandle);
						std::unordered_set<AssetHandle> prefabAssetList = prefab->GetAssetList();
						sceneAssetList.insert(prefabAssetList.begin(), prefabAssetList.end());
					}
				}

				AssetPackFile::SceneInfo& sceneInfo = assetPackFile.indexTable.scenes[sceneHandle];
				for (AssetHandle assetHandle : sceneAssetList)
				{
					AssetPackFile::AssetInfo& assetInfo = sceneInfo.assets[assetHandle];
					const auto& assetMetadata = Project::GetEditorAssetManager()->GetMetadata(assetHandle);
					assetInfo.type = (uint16_t)assetMetadata.type;
				}

				fullAssetList.insert(sceneAssetList.begin(), sceneAssetList.end());
			}
			else
			{
				CONSOLE_LOG_ERROR("Failed to deserialize scene: {} ({})", metadata.filePath, sceneHandle);
			}
		}

		//CONSOLE_LOG_INFO("Project contains {} used assets", fullAssetList.size());

		Buffer appBinary;
		if (std::filesystem::exists(Project::GetScriptModuleFilePath()))
		{
			appBinary = FileSystem::ReadBytes(Project::GetScriptModuleFilePath());
		}

		AssetPackSerializer assetPackSerializer;
		if (aDestination == "")
		{
			aDestination = Project::GetAssetDirectory() / "AssetPack.eap";
		}
		else
		{
			aDestination /= "AssetPack.eap";
		}
		assetPackSerializer.Serialize(aDestination, assetPackFile, appBinary);

		return true;
	}

	std::shared_ptr<AssetPack> AssetPack::Load(const std::filesystem::path& aPath)
	{
		std::shared_ptr<AssetPack> assetPack = std::make_shared<AssetPack>();
		assetPack->myPath = aPath;

		AssetPackSerializer serializer;
		bool success = serializer.DeserializeIndex(assetPack->myPath, assetPack->myFile);
		EPOCH_ASSERT(success);
		if (!success)
		{
			return nullptr;
		}

		// Populate asset handle index
		const auto& index = assetPack->myFile.indexTable;
		for (const auto& [sceneHandle, sceneInfo] : index.scenes)
		{
			assetPack->myAssetHandleIndex.insert(sceneHandle);
			for (const auto& [assetHandle, assetInfo] : sceneInfo.assets)
			{
				assetPack->myAssetHandleIndex.insert(assetHandle);
			}
		}

#if 0
		{
			LOG_INFO("-----------------------------------------------------");
			LOG_INFO("AssetPack Dump {}", assetPack->myPath);
			LOG_INFO("-----------------------------------------------------");
			std::unordered_map<AssetType, uint32_t> typeCounts;
			std::unordered_set<AssetHandle> duplicatePreventionSet;
			for (const auto& [sceneHandle, sceneInfo] : index.scenes)
			{
				LOG_INFO("Scene {}:", sceneHandle);
				for (const auto& [assetHandle, assetInfo] : sceneInfo.assets)
				{
					LOG_INFO("  {} - {}", AssetTypeToString((AssetType)assetInfo.type), assetHandle);

					if (duplicatePreventionSet.find(assetHandle) == duplicatePreventionSet.end())
					{
						duplicatePreventionSet.insert(assetHandle);
						typeCounts[(AssetType)assetInfo.type]++;
					}
				}
			}
			LOG_INFO("-----------------------------------------------------");
			LOG_INFO("Summary:");
			for (const auto& [type, count] : typeCounts)
			{
				LOG_INFO("  {} {}", count, AssetTypeToString(type));
			}
			LOG_INFO("-----------------------------------------------------");
		}
#endif

		return assetPack;
	}

	std::shared_ptr<Scene> AssetPack::LoadScene(AssetHandle aSceneHandle)
	{
		auto it = myFile.indexTable.scenes.find(aSceneHandle);
		if (it == myFile.indexTable.scenes.end())
		{
			return nullptr;
		}

		const AssetPackFile::SceneInfo& sceneInfo = it->second;

		FileStreamReader stream(myPath);
		std::shared_ptr<Scene> scene = AssetImporter::DeserializeSceneFromAssetPack(stream, sceneInfo);
		scene->myHandle = aSceneHandle;
		return scene;
	}

	std::shared_ptr<Asset> AssetPack::LoadAsset(AssetHandle aSceneHandle, AssetHandle aAssetHandle)
	{
		const AssetPackFile::AssetInfo* assetInfo = nullptr;

		bool foundAsset = false;
		if (aSceneHandle)
		{
			// Fast(er) path
			auto it = myFile.indexTable.scenes.find(aSceneHandle);
			if (it != myFile.indexTable.scenes.end())
			{
				const AssetPackFile::SceneInfo& sceneInfo = it->second;
				auto assetIt = sceneInfo.assets.find(aAssetHandle);
				if (assetIt != sceneInfo.assets.end())
				{
					foundAsset = true;
					assetInfo = &assetIt->second;
				}
			}
		}

		if (!foundAsset)
		{
			// Slow(er) path
			for (const auto& [handle, sceneInfo] : myFile.indexTable.scenes)
			{
				auto assetIt = sceneInfo.assets.find(aAssetHandle);
				if (assetIt != sceneInfo.assets.end())
				{
					assetInfo = &assetIt->second;
					break;
				}
			}

			if (!assetInfo)
			{
				return nullptr;
			}
		}

		FileStreamReader stream(myPath);
		std::shared_ptr<Asset> asset = AssetImporter::DeserializeFromAssetPack(stream, *assetInfo);
		if (!asset)
		{
			return nullptr;
		}

		asset->myHandle = aAssetHandle;
		return asset;
	}

	bool AssetPack::IsAssetHandleValid(AssetHandle assetHandle) const
	{
		return myAssetHandleIndex.find(assetHandle) != myAssetHandleIndex.end();
	}

	Buffer AssetPack::ReadAppBinary()
	{
		FileStreamReader stream(myPath);
		stream.SetStreamPosition(myFile.indexTable.packedAppBinaryOffset);
		Buffer buffer;
		stream.ReadBuffer(buffer);
		EPOCH_ASSERT(myFile.indexTable.packedAppBinarySize == (buffer.size + sizeof(uint32_t)));
		return buffer;
	}

	const char* AssetPack::GetLastErrorMessage()
	{
		switch (myLastBuildError)
		{
		case Epoch::AssetPack::BuildError::NoStartScene: return "Start scene not set or not found";
		default: return "";
		}
	}
}
