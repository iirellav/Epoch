#include "epch.h"
#include "Prefab.h"
#include "Epoch/Script/ScriptEngine.h"

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
		myScene->UpdateScriptInstanceEntityReferences(staticDuplicateEntityIDMap);
		staticDuplicateEntityIDMap.clear();
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
