#include "epch.h"
#include "Prefab.h"
#include "Epoch/Script/ScriptEngine.h"
#include "Epoch/Assets/AssetManager.h"

namespace Epoch
{
	Prefab::Prefab()
	{
		myScene = std::make_shared<Scene>("Empty");
	}

	static std::unordered_map<UUID, UUID> staticDuplicateEntityIDMap; //Original entity to prefab entity

	void Prefab::Create(Entity aEntity)
	{
		LOG_INFO("Creating prefab from: {}", aEntity.GetName());
		myScene = std::make_shared<Scene>("Empty");
		myEntity = CreatePrefabFromEntity(aEntity);
		myScene->UpdateEntityReferences(staticDuplicateEntityIDMap);
		staticDuplicateEntityIDMap.clear();
	}

	std::unordered_set<AssetHandle> Prefab::GetAssetList()
	{
		std::unordered_set<AssetHandle> prefabAssetList = myScene->GetAllSceneAssets();

		for (AssetHandle handle : prefabAssetList)
		{
			if (!AssetManager::IsAssetHandleValid(handle))
			{
				continue;
			}

			const auto& metadata = Project::GetEditorAssetManager()->GetMetadata(handle);
			if (metadata.type == AssetType::Prefab)
			{
				std::shared_ptr<Prefab> prefab = AssetManager::GetAsset<Prefab>(handle);
				std::unordered_set<AssetHandle> childPrefabAssetList = prefab->GetAssetList();
				prefabAssetList.insert(childPrefabAssetList.begin(), childPrefabAssetList.end());
			}
		}

		return prefabAssetList;
	}

	Entity Prefab::CreatePrefabFromEntity(Entity aEntity)
	{
		Entity newEntity = myScene->CreateEntity();
		staticDuplicateEntityIDMap.emplace(aEntity.GetUUID(), newEntity.GetUUID());
		newEntity.AddComponent<PrefabComponent>(GetHandle(), newEntity.GetComponent<IDComponent>().id);

		Scene::CopyComponentIfExists<NameComponent>(newEntity, aEntity);
		Scene::CopyComponentsIfExists(AllComponentsDuplicate{}, newEntity, aEntity);

		for (auto childId : aEntity.Children())
		{
			Entity childDuplicate = CreatePrefabFromEntity(aEntity.myScene->GetEntityWithUUID(childId));

			childDuplicate.SetParentUUID(newEntity.GetUUID());
			newEntity.Children().push_back(childDuplicate.GetUUID());
		}
		
		if (newEntity.HasComponent<ScriptComponent>())
		{
			ScriptEngine::DuplicateScriptInstance(aEntity, newEntity);
		}

		return newEntity;
	}
}
