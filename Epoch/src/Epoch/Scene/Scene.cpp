#include "epch.h"
#include "Scene.h"
#include <CommonUtilities/Timer.h>
#include "Prefab.h"
#include "SceneRenderer.h"
#include "Epoch/Rendering/Font.h"
#include "Epoch/Assets/AssetManager.h"
#include "Epoch/Script/ScriptEngine.h"

namespace Epoch
{
	void Scene::CopyTo(std::shared_ptr<Scene> aCopy)
	{
		EPOCH_PROFILE_FUNC();

		aCopy->myName = myName;

		aCopy->myViewportWidth = myViewportWidth;
		aCopy->myViewportHeight = myViewportHeight;

		entt::registry& srcSceneRegistry = myRegistry;
		entt::registry& dstSceneRegistry = aCopy->myRegistry;
		std::unordered_map<UUID, entt::entity> enttMap;

		auto idView = srcSceneRegistry.view<IDComponent>();
		for (auto entity : idView)
		{
			UUID uuid = srcSceneRegistry.get<IDComponent>(entity).id;
			const std::string& name = srcSceneRegistry.get<NameComponent>(entity).name;
			Entity newEntity = aCopy->CreateEntityWithUUID(uuid, name);
			enttMap[uuid] = newEntity;
		}

		CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

		aCopy->SortEntities();
	}

	void Scene::SortEntities()
	{
		myRegistry.sort<IDComponent>([&](const auto lhs, const auto rhs)
			{
				auto lhsEntity = myEntityMap.find(lhs.id);
				auto rhsEntity = myEntityMap.find(rhs.id);
				return static_cast<uint32_t>(lhsEntity->second) < static_cast<uint32_t>(rhsEntity->second);
			});
	}

	Entity Scene::CreateEntity(const std::string& aName)
	{
		return CreateChildEntity({}, aName);
	}

	Entity Scene::CreateChildEntity(Entity aParent, const std::string& aName)
	{
		auto entity = Entity{ myRegistry.create(), this };
		UUID uuid = UUID();

		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<ActiveComponent>();
		entity.AddComponent<NameComponent>(aName);
		entity.AddComponent<RelationshipComponent>();
		entity.AddComponent<TransformComponent>();

		if (aParent)
		{
			ParentEntity(entity, aParent);
		}

		myEntityMap[uuid] = entity;

		SortEntities();

		return entity;
	}

	Entity Scene::CreateEntityWithUUID(UUID aUUID, const std::string& aName)
	{
		Entity entity = { myRegistry.create(), this };

		entity.AddComponent<IDComponent>(aUUID);
		entity.AddComponent<ActiveComponent>();
		entity.AddComponent<NameComponent>(aName);
		entity.AddComponent<RelationshipComponent>();
		entity.AddComponent<TransformComponent>();

		myEntityMap[aUUID] = entity;

		SortEntities();

		return entity;
	}

	void Scene::DestroyEntity(Entity aEntity, bool aFirst)
	{
		if (!aEntity) return;
		if (!myRegistry.valid(aEntity))
		{
			LOG_CRITICAL("Tried to destroy an invalid entity! How did this happen?!");
			return;
		}

		if (aEntity.HasComponent<ScriptComponent>())
		{
			ScriptEngine::ShutdownScriptEntity(aEntity, !myIsPlaying);
		}

		if (myIsPlaying)
		{
			myPhysicsScene->DestroyBody(aEntity);
		}

		if (myOnEntityDestroyedCallback)
		{
			myOnEntityDestroyedCallback(aEntity);
		}

		for (size_t i = 0; i < aEntity.Children().size(); i++)
		{
			auto childId = aEntity.Children()[i];
			Entity child = GetEntityWithUUID(childId);
			DestroyEntity(child, false);
		}

		if (aFirst)
		{
			if (auto parent = aEntity.GetParent(); parent)
			{
				parent.RemoveChild(aEntity);
			}
		}

		myEntityMap.erase(aEntity.GetUUID());
		myRegistry.destroy(aEntity);

		SortEntities();
	}

	void Scene::SubmitToDestroyEntity(Entity aEntity)
	{
		bool isValid = myRegistry.valid((entt::entity)aEntity);
		if (!isValid)
		{
			LOG_WARNING("Trying to destroy invalid entity! entt = {}", (uint32_t)aEntity);
			return;
		}

		SubmitPostUpdateFunc([aEntity]() { aEntity.myScene->DestroyEntity(aEntity); });
	}
	
	static std::unordered_map<UUID, UUID> staticDuplicateEntityIDMap; //original to new

	Entity Scene::DuplicateEntity(Entity aEntity, bool aFirst)
	{
		Entity newEntity = CreateEntity(aEntity.GetName());
		staticDuplicateEntityIDMap.emplace(aEntity.GetUUID(), newEntity.GetUUID());

		CopyComponentsIfExists(AllComponentsDuplicate{}, newEntity, aEntity);

		auto childIds = aEntity.Children(); // need to take a copy of children here, because the collection is mutated below
		for (UUID childId : childIds)
		{
			Entity childDuplicate = DuplicateEntity(GetEntityWithUUID(childId), false);

			// At this point childDuplicate is a child of entity, we need to remove it from that entity
			UnparentEntity(childDuplicate, false);

			childDuplicate.SetParentUUID(newEntity.GetUUID());
			newEntity.Children().push_back(childDuplicate.GetUUID());
		}

		Entity parent = aEntity.GetParent();
		if (parent)
		{
			newEntity.SetParentUUID(parent.GetUUID());
			parent.Children().push_back(newEntity.GetUUID());
		}

		FindBoneEntityIds(newEntity);

		if (newEntity.HasComponent<ScriptComponent>())
		{
			ScriptEngine::DuplicateScriptInstance(aEntity, newEntity);
		}

		if (aFirst)
		{
			UpdatePrefabInstanceEntityReferences(staticDuplicateEntityIDMap);
			staticDuplicateEntityIDMap.clear();
		}

		return newEntity;
	}

	Entity Scene::Instantiate(std::shared_ptr<Prefab> aPrefab, const CU::Vector3f* aTranslation, const CU::Vector3f* aRotation, const CU::Vector3f* aScale)
	{
		return InstantiateChild(aPrefab, {}, aTranslation, aRotation, aScale);

		//EPOCH_PROFILE_FUNC();
		//
		//Entity result;
		//
		//if (aPrefab->myEntity)
		//{
		//	result = CreatePrefabEntity(aPrefab->myEntity, {});
		//}
		//
		//FindBoneEntityIds(result);
		//
		//auto& tc = result.GetComponent<TransformComponent>();
		//if (aTranslation) tc.transform.SetTranslation(*aTranslation);
		//if (aRotation) tc.transform.SetRotation(*aRotation);
		//if (aScale) tc.transform.SetScale(*aScale);
		//
		//return result;
	}

	Entity Scene::InstantiateChild(std::shared_ptr<Prefab> aPrefab, Entity aParent, const CU::Vector3f* aTranslation, const CU::Vector3f* aRotation, const CU::Vector3f* aScale)
	{
		EPOCH_PROFILE_FUNC();

		Entity result;

		if (aPrefab->myEntity)
		{
			result = CreatePrefabEntity(aPrefab->myEntity, aParent);
		}

		auto& tc = result.GetComponent<TransformComponent>();
		if (aTranslation) tc.transform.SetTranslation(*aTranslation);
		if (aRotation) tc.transform.SetRotation(*aRotation);
		if (aScale) tc.transform.SetScale(*aScale);

		for (auto [orgID, newID] : staticDuplicateEntityIDMap)
		{
			if (myIsPlaying)
			{
				Entity entity = GetEntityWithUUID(newID);
				myPhysicsScene->CreateBody(entity);
			}
		}

		UpdatePrefabInstanceEntityReferences(staticDuplicateEntityIDMap);
		staticDuplicateEntityIDMap.clear();

		return result;
	}

	Entity Scene::CreatePrefabEntity(Entity aEntity, Entity aParent)
	{
		Entity newEntity = CreateEntity();
		staticDuplicateEntityIDMap.emplace(aEntity.GetUUID(), newEntity.GetUUID());

		if (aParent)
		{
			newEntity.SetParent(aParent);
		}

		CopyComponentIfExists<NameComponent>(newEntity, aEntity);
		CopyComponentsIfExists(AllComponentsDuplicate{}, newEntity, aEntity);

		for (auto childId : aEntity.Children())
		{
			CreatePrefabEntity(aEntity.myScene->GetEntityWithUUID(childId), newEntity);
		}

		if (newEntity.HasComponent<ScriptComponent>())
		{
			ScriptEngine::DuplicateScriptInstance(aEntity, newEntity);
		}

		return newEntity;
	}

	Entity Scene::InstantiateMesh(std::shared_ptr<Mesh> aMesh)
	{
		auto& assetData = Project::GetEditorAssetManager()->GetMetadata(aMesh->GetHandle());
		Entity rootEntity = CreateEntity(assetData.filePath.stem().string());

		if (aMesh->HasSkeleton())
		{
			auto& smrc = rootEntity.AddComponent<SkinnedMeshRendererComponent>(aMesh->GetHandle());
			BuildMeshEntityHierarchy(rootEntity, aMesh, aMesh->GetRootNode());
			smrc.boneEntityIds = FindBoneEntityIds(rootEntity, aMesh);
		}
		else
		{
			auto& mrc = rootEntity.AddComponent<MeshRendererComponent>(aMesh->GetHandle());
		}

		return rootEntity;
	}

	void Scene::UpdatePrefabInstanceEntityReferences(const std::unordered_map<UUID, UUID>& aPrefabEntityIDMap)
	{
		for (auto [orgID, newID] : aPrefabEntityIDMap)
		{
			Entity entity = GetEntityWithUUID(newID);

			if (!entity.HasComponent<ScriptComponent>())
			{
				continue;
			}

			auto& sc = entity.GetComponent<ScriptComponent>();

			if (!ScriptEngine::IsModuleValid(sc.scriptClassHandle))
			{
				continue;
			}

			for (auto fieldID : sc.fieldIDs)
			{
				FieldInfo* field = ScriptCache::GetFieldByID(fieldID);

				if (field->type != FieldType::Entity)
				{
					continue;
				}

				std::shared_ptr<FieldStorageBase> storage = ScriptEngine::GetFieldStorage(entity, field->id);

				if (field->IsArray())
				{
					auto fieldStorage = std::dynamic_pointer_cast<ArrayFieldStorage>(storage);

					for (size_t i = 0; i < fieldStorage->GetLength(); i++)
					{
						UUID oldEntityID = fieldStorage->GetValue<UUID>((uint32_t)i);

						if (aPrefabEntityIDMap.find(oldEntityID) != aPrefabEntityIDMap.end())
						{
							UUID newEntityID = aPrefabEntityIDMap.at(oldEntityID);
							fieldStorage->SetValue((uint32_t)i, newEntityID);
						}
					}
				}
				else
				{
					auto fieldStorage = std::dynamic_pointer_cast<FieldStorage>(storage);

					UUID oldEntityID = fieldStorage->GetValue<UUID>();

					if (aPrefabEntityIDMap.find(oldEntityID) != aPrefabEntityIDMap.end())
					{
						UUID newEntityID = aPrefabEntityIDMap.at(oldEntityID);
						fieldStorage->SetValue(newEntityID);
					}
				}
			}
		}
	}

	void Scene::OnRuntimeStart()
	{
		myIsPlaying = true;

		myPhysicsScene = PhysicsSystem::CreatePhysicsScene(this);
		ScriptEngine::InitializeRuntime();

		for (const auto& [entityID, entityInstance] : ScriptEngine::GetEntityInstances())
		{
			if (myEntityMap.find(entityID) != myEntityMap.end())
			{
				Entity entity{ myEntityMap[entityID], this };

				if (ScriptEngine::IsEntityInstantiated(entity))
				{
					ScriptEngine::CallMethod(entityInstance, "OnStart");
				}
			}
		}
	}

	void Scene::OnRuntimeStop()
	{
		myPhysicsScene->Destroy();
		myPhysicsScene = nullptr;

		for (const auto& [entityID, entityInstance] : ScriptEngine::GetEntityInstances())
		{
			if (myEntityMap.find(entityID) != myEntityMap.end())
			{
				Entity entity{ myEntityMap[entityID], this };

				if (ScriptEngine::IsEntityInstantiated(entity))
				{
					ScriptEngine::CallMethod(entityInstance, "OnEnd");
				}
			}
		}
		
		ScriptEngine::ShutdownRuntime();

		myIsPlaying = false;
	}

	void Scene::OnSimulationStart()
	{
		myIsSimulating = true;

		myPhysicsScene = PhysicsSystem::CreatePhysicsScene(this);
		ScriptEngine::InitializeRuntime();
	}

	void Scene::OnSimulationStop()
	{
		myPhysicsScene->Destroy();
		myPhysicsScene = nullptr;

		myIsSimulating = false;
	}

	void Scene::OnUpdateRuntime()
	{
		EPOCH_PROFILE_FUNC();

		if (!myIsPaused || ShouldStep())
		{
			// Update scripts
			{
				ScriptEngine::UpdateDeltaTime();

				{
					EPOCH_PROFILE_SCOPE("Scene::OnUpdate - C# OnUpdate");
					Timer timer;

					for (const auto& [entityID, entityInstance] : ScriptEngine::GetEntityInstances())
					{
						if (myEntityMap.find(entityID) != myEntityMap.end())
						{
							Entity entity{ myEntityMap[entityID], this };
							ScriptComponent sc = entity.GetComponent<ScriptComponent>();

							if (entity.IsActive() && sc.isActive && ScriptEngine::IsEntityInstantiated(entity))
							{
								ScriptEngine::CallMethod(entityInstance, "OnUpdate");
							}
						}
					}

					myPerformanceTimers.scriptUpdate = timer.ElapsedMillis();
				}

				{
					EPOCH_PROFILE_SCOPE("Scene::OnLateUpdate - C# OnLateUpdate");
					Timer timer;

					for (const auto& [entityID, entityInstance] : ScriptEngine::GetEntityInstances())
					{
						if (myEntityMap.find(entityID) != myEntityMap.end())
						{
							Entity entity{ myEntityMap[entityID], this };

							if (entity.IsActive() && ScriptEngine::IsEntityInstantiated(entity))
							{
								ScriptEngine::CallMethod(entityInstance, "OnLateUpdate");
							}
						}
					}

					myPerformanceTimers.scriptLateUpdate = timer.ElapsedMillis();
				}
			}
			
			// Update Physics
			{
				Timer timer;
				myPhysicsScene->Simulate();
				myPerformanceTimers.physicsSimulation = timer.ElapsedMillis();
			}

			// Update animation
			// Update audio

			for (auto&& fn : myPostUpdateQueue)
			{
				fn();
			}
			myPostUpdateQueue.clear();
		}
	}

	void Scene::OnUpdateEditor()
	{
		EPOCH_PROFILE_FUNC();

		if (myIsSimulating)
		{
			if (!myIsPaused || ShouldStep())
			{
				// Update physics
				{
					Timer timer;
					myPhysicsScene->Simulate();
					myPerformanceTimers.physicsSimulation = timer.ElapsedMillis();
				}
			}
		}
	}

	void Scene::OnRenderRuntime(std::shared_ptr<SceneRenderer> aRenderer)
	{
		Entity cameraEntity = GetPrimaryCameraEntity();
		if (!cameraEntity) return;

		CU::Transform worlTrans = GetWorldSpaceTransform(cameraEntity);
		const CU::Matrix4x4f cameraViewMatrix = worlTrans.GetMatrix().GetFastInverse();
		SceneCamera& camera = cameraEntity.GetComponent<CameraComponent>().camera;
		camera.SetViewportSize(myViewportWidth, myViewportHeight);

		const SceneRendererCamera renderCamera
		(
			(Camera)camera,
			worlTrans.GetTranslation(),
			cameraViewMatrix,
			camera.GetPerspectiveNearPlane(),
			camera.GetPerspectiveFarPlane(),
			camera.GetPerspectiveFOV()
		);
		RenderScene(aRenderer, renderCamera, true);
	}

	void Scene::OnRenderEditor(std::shared_ptr<SceneRenderer> aRenderer, EditorCamera& aCamera)
	{
		const SceneRendererCamera renderCamera
		(
			(Camera)aCamera,
			aCamera.GetTransform().GetTranslation(),
			aCamera.GetViewMatrix(),
			aCamera.GetNearPlane(),
			aCamera.GetFarPlane(),
			aCamera.GetFOV()
		);
		RenderScene(aRenderer, renderCamera, false);
	}

	void Scene::OnSceneTransition(AssetHandle aScene)
	{
		if (myOnSceneTransitionCallback)
		{
			myOnSceneTransitionCallback(aScene);
		}
		else
		{
			LOG_WARNING("Cannot transition scene - no callback set!");
		}
	}

	void Scene::SetViewportSize(uint32_t aWidth, uint32_t aHeight)
	{
		myViewportWidth = aWidth;
		myViewportHeight = aHeight;
	}

	Entity Scene::TryGetEntityByName(std::string_view aName)
	{
		EPOCH_PROFILE_FUNC();

		auto entities = GetAllEntitiesWith<NameComponent>();
		for (auto entity : entities)
		{
			if (entities.get<NameComponent>(entity).name == aName)
			{
				return Entity{ entity, this };
			}
		}

		return Entity{};
	}

	std::vector<Entity> Scene::GetEntitiesByName(std::string_view aName)
	{
		EPOCH_PROFILE_FUNC();

		std::vector<Entity> matches;

		auto entities = GetAllEntitiesWith<NameComponent>();
		for (auto entity : entities)
		{
			if (entities.get<NameComponent>(entity).name == aName)
			{
				matches.emplace_back(entity, this);
			}
		}

		return matches;
	}

	Entity Scene::GetEntityWithUUID(UUID aUUID)
	{
		EPOCH_ASSERT(myEntityMap.find(aUUID) != myEntityMap.end(), "Entity doesn't exist in scene!");
		return Entity{ myEntityMap.at(aUUID), this };
	}

	Entity Scene::TryGetEntityWithUUID(UUID aUUID)
	{
		if (myEntityMap.find(aUUID) != myEntityMap.end())
		{
			return Entity{ myEntityMap.at(aUUID), this };
		}

		return Entity{};
	}

	Entity Scene::TryGetDescendantEntityWithName(Entity aEntity, const std::string& aName)
	{
		if (aEntity)
		{
			if (aEntity.GetComponent<NameComponent>().name == aName)
			{
				return aEntity;
			}

			for (const auto childId : aEntity.Children())
			{
				Entity descendant = TryGetDescendantEntityWithName(GetEntityWithUUID(childId), aName);
				if (descendant)
				{
					return descendant;
				}
			}
		}
		return {};
	}

	bool Scene::IsEntityValid(Entity aEntity) const
	{
		return myRegistry.valid(aEntity);
	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = myRegistry.view<CameraComponent>();
		for (auto entity : view)
		{
			Entity camEntity(entity, this);
			if (!camEntity.IsActive()) continue;

			const auto& camera = view.get<CameraComponent>(entity);
			if (!camera.isActive) continue;

			if (camera.primary)
			{
				return camEntity;
			}
		}

		return Entity{};
	}

	std::pair<uint32_t, uint32_t> Scene::GetPhysicsBodyCount()
	{
		EPOCH_PROFILE_FUNC();

		if ((myIsPlaying || myIsSimulating) && myPhysicsScene)
		{
			return { myPhysicsScene->GetStaticPhysicsBodyCount(), myPhysicsScene->GetDynamicPhysicsBodyCount() };
		}
		else
		{
			uint32_t staticCount = 0, dynamicCount = 0;

			std::vector<UUID> compoundedEntities;
			std::vector<UUID> compoundedChildEntities;
			std::vector<UUID> other;

			auto view = myRegistry.view<TransformComponent>();
			for (auto enttID : view)
			{
				Entity entity(enttID, this);

				if (!entity.IsActive()) continue;

				bool isCompound = false;
				if (entity.HasComponent<RigidbodyComponent>() && !entity.HasAny<BoxColliderComponent, SphereColliderComponent, CapsuleColliderComponent>())
				{
					isCompound = true;
					compoundedEntities.push_back(entity.GetUUID());

					for (UUID child : entity.Children())
					{
						Entity childEntity = GetEntityWithUUID(child);
						if (childEntity.HasAny<BoxColliderComponent, SphereColliderComponent, CapsuleColliderComponent>())
						{
							compoundedChildEntities.push_back(child);
						}
					}

					continue;
				}

				if (entity.HasAny<BoxColliderComponent, SphereColliderComponent, CapsuleColliderComponent>())
				{
					other.push_back(entity.GetUUID());
				}
			}

			for (UUID entityID : compoundedEntities)
			{
				++dynamicCount;
			}

			for (UUID entityID : other)
			{
				if (std::find(compoundedChildEntities.begin(), compoundedChildEntities.end(), entityID) != compoundedChildEntities.end())
				{
					continue;
				}
				
				Entity entity = GetEntityWithUUID(entityID);
				if (entity.HasComponent<RigidbodyComponent>())
				{
					++dynamicCount;
				}
				else
				{
					++staticCount;
				}
			}

			return { staticCount, dynamicCount };
		}
	}

	CU::Matrix4x4f Scene::GetWorldSpaceTransformMatrix(Entity aEntity)
	{
		CU::Matrix4x4f transform = CU::Matrix4x4f::Identity;

		Entity parent = TryGetEntityWithUUID(aEntity.GetParentUUID());
		if (parent)
		{
			transform = GetWorldSpaceTransformMatrix(parent);
		}

		return aEntity.TransformMatrix() * transform;
	}

	CU::Transform Scene::GetWorldSpaceTransform(Entity aEntity)
	{
		const CU::Matrix4x4f transformMatrix = GetWorldSpaceTransformMatrix(aEntity);
		CU::Transform transform;
		transform.SetTransform(transformMatrix);
		return transform;
	}

	void Scene::ConvertToLocalSpace(Entity aEntity)
	{
		//EPOCH_PROFILE_FUNC();

		Entity parent = TryGetEntityWithUUID(aEntity.GetParentUUID());
		if (!parent) return;

		auto& transform = aEntity.Transform();
		const CU::Matrix4x4f parentTransform = GetWorldSpaceTransformMatrix(parent);
		const CU::Matrix4x4f localTransform = transform.GetMatrix() * parentTransform.GetFastInverse();

		transform.SetTransform(localTransform);
	}

	void Scene::ConvertToWorldSpace(Entity aEntity)
	{
		//EPOCH_PROFILE_FUNC();

		Entity parent = TryGetEntityWithUUID(aEntity.GetParentUUID());
		if (!parent) return;

		CU::Matrix4x4f transform = GetWorldSpaceTransformMatrix(aEntity);
		auto& entityTransform = aEntity.Transform();
		entityTransform.SetTransform(transform);
	}

	void Scene::ParentEntity(Entity aEntity, Entity aParent)
	{
		EPOCH_PROFILE_FUNC();

		if (aParent.IsDescendantOf(aEntity))
		{
			UnparentEntity(aParent);

			Entity newParent = TryGetEntityWithUUID(aEntity.GetParentUUID());
			if (newParent)
			{
				UnparentEntity(aEntity);
				ParentEntity(aParent, newParent);
			}
		}
		else
		{
			Entity previousParent = TryGetEntityWithUUID(aEntity.GetParentUUID());
			if (previousParent)
			{
				UnparentEntity(aEntity);
			}
		}

		aEntity.SetParentUUID(aParent.GetUUID());
		aParent.Children().push_back(aEntity.GetUUID());

		ConvertToLocalSpace(aEntity);
	}

	void Scene::UnparentEntity(Entity aEntity, bool aConvertToWorldSpace)
	{
		EPOCH_PROFILE_FUNC();

		Entity parent = TryGetEntityWithUUID(aEntity.GetParentUUID());
		if (!parent) return;

		auto& children = parent.Children();
		children.erase(std::remove(children.begin(), children.end(), aEntity.GetUUID()), children.end());

		if (aConvertToWorldSpace)
		{
			ConvertToWorldSpace(aEntity);
		}

		aEntity.SetParentUUID(0);
	}

	void Scene::BuildMeshEntityHierarchy(Entity aParent, std::shared_ptr<Mesh> aMesh, const MeshNode& aNode)
	{
		const auto& nodes = aMesh->GetNodes();

		// Skip empty root node
		if (aNode.IsRoot() && aNode.submeshes.size() == 0)
		{
			for (uint32_t child : aNode.children)
			{
				BuildMeshEntityHierarchy(aParent, aMesh, nodes[child]);
			}

			return;
		}

		if (aNode.submeshes.size() == 0)
		{
			Entity nodeEntity = CreateChildEntity(aParent, aNode.name);
			nodeEntity.Transform().SetTransform(aNode.localTransform);

			for (uint32_t child : aNode.children)
			{
				BuildMeshEntityHierarchy(nodeEntity, aMesh, nodes[child]);
			}
		}
	}

	void Scene::FindBoneEntityIds(Entity aRoot)
	{
		if (!aRoot.HasComponent<SkinnedMeshRendererComponent>())
		{
			return;
		}

		auto& smrc = aRoot.GetComponent<SkinnedMeshRendererComponent>();
		auto mesh = AssetManager::GetAssetAsync<Mesh>(smrc.mesh);
		if (mesh)
		{
			smrc.boneEntityIds = FindBoneEntityIds(aRoot, mesh);
		}
	}

	std::vector<UUID> Scene::FindBoneEntityIds(Entity aRoot, std::shared_ptr<Mesh> aMesh)
	{
		EPOCH_PROFILE_FUNC();

		std::vector<UUID> boneEntityIds;

		const auto& bones = aMesh->GetSkeleton()->GetBones();

		for (const auto& bone : bones)
		{
			Entity boneEntity = TryGetDescendantEntityWithName(aRoot, bone.name);
			if (boneEntity)
			{
				boneEntityIds.emplace_back(boneEntity.GetUUID());
			}
		}

		return boneEntityIds;
	}

	std::vector<CU::Matrix4x4f> Scene::GetModelSpaceBoneTransforms(Entity aEntity, std::shared_ptr<Mesh> aMesh)
	{
		EPOCH_PROFILE_FUNC();

		std::vector<CU::Matrix4x4f> boneTransforms;

		if (aMesh->HasSkeleton())
		{
			auto skeleton = aMesh->GetSkeleton();

			auto& smrc = aEntity.GetComponent<SkinnedMeshRendererComponent>();

			if (smrc.boneEntityIds.empty())
			{
				smrc.boneEntityIds = FindBoneEntityIds(aEntity, aMesh);
			}

			if (smrc.boneEntityIds.size() != skeleton->GetNumBones())
			{
				boneTransforms.resize(skeleton->GetNumBones());
				return boneTransforms;
			}

			boneTransforms.resize(smrc.boneEntityIds.size());
			GetModelSpaceBoneTransform(smrc.boneEntityIds, boneTransforms, 0, CU::Matrix4x4f::Identity, skeleton);
		}

		return boneTransforms;
	}

	void Scene::GetModelSpaceBoneTransform(const std::vector<UUID>& aBoneEntityIds, std::vector<CU::Matrix4x4f>& outBoneTransforms, uint32_t aBoneIndex, const CU::Matrix4x4f& aParentTransform, std::shared_ptr<Skeleton> aSkeleton)
	{
		const Skeleton::Bone& bone = aSkeleton->GetBone(aBoneIndex);

		auto boneEntity = TryGetEntityWithUUID(aBoneEntityIds[aBoneIndex]);
		const CU::Matrix4x4f localTransform = boneEntity ? boneEntity.GetComponent<TransformComponent>().GetMatrix() : CU::Matrix4x4f::Identity;

		const CU::Matrix4x4f matrix = localTransform * aParentTransform;

		for (size_t i = 0; i < bone.childrenIndices.size(); i++)
		{
			GetModelSpaceBoneTransform(aBoneEntityIds, outBoneTransforms, bone.childrenIndices[i], matrix, aSkeleton);
		}

		outBoneTransforms[aBoneIndex] = bone.invBindPose * matrix;
	}

	void Scene::RenderScene(std::shared_ptr<SceneRenderer> aRenderer, const SceneRendererCamera& aCamera, bool aIsRuntime)
	{
		EPOCH_PROFILE_FUNC();

		std::unordered_map<AssetHandle, std::shared_ptr<Asset>> assetAccelerationMap;

		aRenderer->BeginScene(aCamera);

		{
			EPOCH_PROFILE_SCOPE("Scene::RenderScene::UpdateLightEnvironment");

			myLightEnvironment = LightEnvironment();
			
			auto directionalLights = GetAllEntitiesWith<DirectionalLightComponent>();
			for (auto entityID : directionalLights)
			{
				Entity entity = Entity(entityID, this);
				if (!entity.IsActive()) continue;

				const auto& dlc = directionalLights.get<DirectionalLightComponent>(entityID);
				if (!dlc.isActive && dlc.intensity > 0.0f) continue;

				const CU::Transform transform = GetWorldSpaceTransform(entity);

				myLightEnvironment.directionalLight.direction = transform.GetForward();
				myLightEnvironment.directionalLight.color = dlc.color.GetVector3();
				myLightEnvironment.directionalLight.intensity = dlc.intensity;

				break;
			}

			auto skyLights = GetAllEntitiesWith<SkyLightComponent>();
			for (auto entityID : skyLights)
			{
				Entity entity = Entity(entityID, this);
				if (!entity.IsActive()) continue;

				const auto& slc = skyLights.get<SkyLightComponent>(entityID);
				if (!slc.isActive && slc.intensity > 0.0f) continue;

				std::shared_ptr<Environment> environment = AssetManager::GetAsset<Environment>(slc.environment);

				if (!environment) continue;

				myLightEnvironment.environment = environment;
				myLightEnvironment.environmentIntensity = slc.intensity;

				break;
			}

			auto pointLights = GetAllEntitiesWith<PointLightComponent>();
			myLightEnvironment.pointLights.resize(pointLights.size());
			uint32_t pointLightIndex = 0;
			for (auto entityID : pointLights)
			{
				Entity entity = Entity(entityID, this);
				if (!entity.IsActive()) continue;

				const auto& plc = pointLights.get<PointLightComponent>(entityID);
				if (!plc.isActive && plc.intensity > 0.0f) continue;

				CU::Transform transform = GetWorldSpaceTransform(entity);

				PointLight& pl = myLightEnvironment.pointLights[pointLightIndex++];
				const auto projMatrix = CU::Matrix4x4f::CreatePerspectiveProjection(90.f * CU::Math::ToRad, 10.f, pl.range, 1.0f);
				pl.viewProjection = transform.GetMatrix().GetFastInverse() * projMatrix;
				pl.position = transform.GetTranslation();
				pl.color = plc.color.GetVector3();
				pl.intensity = plc.intensity;
				pl.range = CU::Math::Max(plc.range, 0.0001f);
				pl.cookie = plc.cookieTexture;
			}

			auto spotlights = GetAllEntitiesWith<SpotlightComponent>();
			myLightEnvironment.spotlights.resize(spotlights.size());
			uint32_t spotlightIndex = 0;
			for (auto entityID : spotlights)
			{
				Entity entity = Entity(entityID, this);
				if (!entity.IsActive()) continue;

				const auto& slc = spotlights.get<SpotlightComponent>(entityID);
				if (!slc.isActive && slc.intensity > 0.0f) continue;

				CU::Transform transform = GetWorldSpaceTransform(entity);

				Spotlight& sl = myLightEnvironment.spotlights[spotlightIndex++];
				const auto projMatrix = CU::Matrix4x4f::CreatePerspectiveProjection(slc.outerSpotAngle * 2.0f, 10.f, slc.range, 1.0f);
				sl.viewProjection = transform.GetMatrix().GetFastInverse() * projMatrix;
				sl.position = transform.GetTranslation();
				sl.direction = transform.GetForward();
				sl.color = slc.color.GetVector3();
				sl.intensity = slc.intensity;
				sl.range = CU::Math::Max(slc.range, 0.0001f);
				sl.coneAngle = std::cos(slc.outerSpotAngle);
				sl.coneAngleDiff = std::cos(slc.innerSpotAngle) - sl.coneAngle;
				sl.cookie = slc.cookieTexture;
			}
		}

		{
			EPOCH_PROFILE_SCOPE("Scene::RenderScene::UpdatePostProcessingData");

			myPostProcessingData = PostProcessingData();
			
			auto volumes = GetAllEntitiesWith<VolumeComponent>();
			for (auto entityID : volumes)
			{
				Entity entity = Entity(entityID, this);
				if (!entity.IsActive()) continue;

				const auto& vc = volumes.get<VolumeComponent>(entityID);
				if (!vc.isActive) continue;

				if (vc.tonemapping.enabled)
				{
					myPostProcessingData.bufferData.tonemap = vc.tonemapping.tonemap;
				}
				
				myPostProcessingData.bufferData.flags |= (vc.colorGrading.enabled << 0);
				if (vc.colorGrading.enabled)
				{
					myPostProcessingData.colorGradingLUT = vc.colorGrading.lut;
				}
				
				myPostProcessingData.bufferData.flags |= (vc.vignette.enabled << 1);
				if (vc.vignette.enabled)
				{
					myPostProcessingData.bufferData.vignetteCenter = vc.vignette.center;
					myPostProcessingData.bufferData.vignetteColor = vc.vignette.color;
					myPostProcessingData.bufferData.vignetteIntensity = vc.vignette.intensity;
					myPostProcessingData.bufferData.vignetteSize = vc.vignette.size;
					myPostProcessingData.bufferData.vignetteSmoothness = vc.vignette.smoothness;
				}

				myPostProcessingData.bufferData.flags |= (vc.distanceFog.enabled << 2);
				if (vc.distanceFog.enabled)
				{
					myPostProcessingData.bufferData.distanceFogColor = vc.distanceFog.color.GetVector3();
					myPostProcessingData.bufferData.distanceFogDensity = vc.distanceFog.density;
					myPostProcessingData.bufferData.distanceFogOffset = vc.distanceFog.offset;
				}

				myPostProcessingData.bufferData.flags |= (vc.posterization.enabled << 3);
				if (vc.posterization.enabled)
				{
					myPostProcessingData.bufferData.posterizationSteps = vc.posterization.steps;
				}

				break;
			}
		}

		{
			EPOCH_PROFILE_SCOPE("Scene::RenderScene::SubmitMeshes");

			{
				auto view = GetAllEntitiesWith<MeshRendererComponent>();
				for (auto id : view)
				{
					Entity entity = Entity(id, this);
					if (!entity.IsActive()) continue;

					const auto& mrc = view.get<MeshRendererComponent>(id);
					if (!mrc.isActive) continue;

					std::shared_ptr<Mesh> mesh;
					if (assetAccelerationMap.find(mrc.mesh) != assetAccelerationMap.end())
					{
						mesh = std::static_pointer_cast<Mesh>(assetAccelerationMap[mrc.mesh]);
					}
					else
					{
						mesh = AssetManager::GetAssetAsync<Mesh>(mrc.mesh);
						assetAccelerationMap[mrc.mesh] = mesh;
					}

					if (mesh)
					{
						CU::Matrix4x4f transform = GetWorldSpaceTransformMatrix(entity);
						aRenderer->SubmitMesh(mesh, mrc.materialTable, transform, (uint32_t)entity);
					}
				}
			}

			{
				auto view = GetAllEntitiesWith<SkinnedMeshRendererComponent>();
				for (auto id : view)
				{
					Entity entity = Entity(id, this);
					if (!entity.IsActive()) continue;

					const auto& smrc = view.get<SkinnedMeshRendererComponent>(id);
					if (!smrc.isActive) continue;

					std::shared_ptr<Mesh> mesh;
					if (assetAccelerationMap.find(smrc.mesh) != assetAccelerationMap.end())
					{
						mesh = std::static_pointer_cast<Mesh>(assetAccelerationMap[smrc.mesh]);
					}
					else
					{
						mesh = AssetManager::GetAssetAsync<Mesh>(smrc.mesh);
						assetAccelerationMap[smrc.mesh] = mesh;
					}

					if (mesh)
					{
						CU::Matrix4x4f transform = GetWorldSpaceTransformMatrix(entity);
						aRenderer->SubmitAnimatedMesh(mesh, transform, GetModelSpaceBoneTransforms(entity, mesh));
					}
				}
			}
		}

		{
			EPOCH_PROFILE_SCOPE("Scene::RenderScene::SubmitSprites");

			auto view = GetAllEntitiesWith<SpriteRendererComponent>();
			for (auto id : view)
			{
				Entity entity = Entity(id, this);
				if (!entity.IsActive()) continue;

				const auto& src = view.get<SpriteRendererComponent>(id);
				if (!src.isActive) continue;

				std::shared_ptr<Texture2D> texture;
				if (assetAccelerationMap.find(src.texture) != assetAccelerationMap.end())
				{
					texture = std::static_pointer_cast<Texture2D>(assetAccelerationMap[src.texture]);
				}
				else
				{
					//texture = AssetManager::GetAssetAsync<Texture2D>(src.texture);
					texture = AssetManager::GetAsset<Texture2D>(src.texture); //TODO: Make async
					if (texture) assetAccelerationMap[src.texture] = texture;
				}

				CU::Matrix4x4f transform = GetWorldSpaceTransformMatrix(entity);
				aRenderer->SubmitQuad(transform, texture, src.tint, src.flipX, src.flipY, (uint32_t)entity);
			}
		}

		{
			EPOCH_PROFILE_SCOPE("Scene::RenderScene::SubmitText");

			auto view = GetAllEntitiesWith<TextRendererComponent>();
			for (auto id : view)
			{
				Entity entity = Entity(id, this);
				if (!entity.IsActive()) continue;

				const auto& trc = view.get<TextRendererComponent>(id);
				if (!trc.isActive) continue;

				CU::Matrix4x4f transform = GetWorldSpaceTransformMatrix(entity);
				std::shared_ptr<Font> font;
				if (assetAccelerationMap.find(trc.font) != assetAccelerationMap.end())
				{
					font = std::static_pointer_cast<Font>(assetAccelerationMap[trc.font]);
				}
				else
				{
					font = AssetManager::GetAssetAsync<Font>(trc.font);
					if (font) assetAccelerationMap[trc.font] = font;
				}

				if (!font)
				{
					font = Font::GetDefaultFont();
				}

				SceneRenderer::TextSettings settings;
				settings.centered = trc.centered;
				settings.maxWidth = trc.maxWidth;
				settings.color = trc.color;
				settings.lineHeightOffset = trc.lineSpacing;
				settings.letterSpacing = trc.letterSpacing;
				aRenderer->SubmitText(trc.text, font, transform, settings, (uint32_t)entity);
			}
		}

		aRenderer->EndScene();
	}
}
