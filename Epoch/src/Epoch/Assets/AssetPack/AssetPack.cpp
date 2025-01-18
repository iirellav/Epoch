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

	bool AssetPack::CreateFromActiveProject(std::atomic<float>& aProgress)
	{
		AssetPackFile assetPackFile;
		assetPackFile.header.buildVersion = Platform::GetCurrentDateTimeU64();

		aProgress = 0.0f;

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
		aProgress += 0.1f;

		float progressIncrement = 0.4f / (float)referencedScenes.size();

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

			aProgress += progressIncrement;
		}

		CONSOLE_LOG_INFO("Project contains {} used assets", fullAssetList.size());

		return true;

		Buffer appBinary;
		if (std::filesystem::exists(Project::GetScriptModuleFilePath()))
		{
			appBinary = FileSystem::ReadBytes(Project::GetScriptModuleFilePath());
		}

		AssetPackSerializer assetPackSerializer;
		assetPackSerializer.Serialize(Project::GetAssetDirectory() / "AssetPack.eap", assetPackFile, appBinary, aProgress);

		aProgress = 1.0f;
		return true;
	}

	std::shared_ptr<AssetPack> AssetPack::LoadActiveProject()
	{
		return std::shared_ptr<AssetPack>();
	}

	const char* AssetPack::GetLastErrorMessage()
	{
		return nullptr;
	}
}
