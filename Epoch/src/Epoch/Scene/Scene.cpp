#include "epch.h"
#include "Scene.h"
#include <CommonUtilities/Timer.h>
#include "Epoch/Core/Input.h"
#include "Prefab.h"
#include "SceneRenderer.h"
#include "Epoch/Rendering/Font.h"
#include "Epoch/Assets/AssetManager.h"
#include "Epoch/Script/ScriptEngine.h"
#include "Epoch/Rendering/Renderer.h"
#include "Epoch/Rendering/DebugRenderer.h"

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
			UpdateScriptInstanceEntityReferences(staticDuplicateEntityIDMap);
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

		UpdateScriptInstanceEntityReferences(staticDuplicateEntityIDMap);
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

	void Scene::UpdateScriptInstanceEntityReferences(const std::unordered_map<UUID, UUID>& aEntityIDMap)
	{
		for (auto [orgID, newID] : aEntityIDMap)
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

						if (aEntityIDMap.find(oldEntityID) != aEntityIDMap.end())
						{
							UUID newEntityID = aEntityIDMap.at(oldEntityID);
							fieldStorage->SetValue((uint32_t)i, newEntityID);
						}
					}
				}
				else
				{
					auto fieldStorage = std::dynamic_pointer_cast<FieldStorage>(storage);

					UUID oldEntityID = fieldStorage->GetValue<UUID>();

					if (aEntityIDMap.find(oldEntityID) != aEntityIDMap.end())
					{
						UUID newEntityID = aEntityIDMap.at(oldEntityID);
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

					auto entities = GetAllEntitiesWith<ScriptComponent>();
					for (auto id : entities)
					{
						Entity entity = Entity(id, this);
						const auto& sc = entities.get<ScriptComponent>(id);
						if (entity.IsActive() && sc.isActive && sc.IsFlagSet(ManagedClassMethodFlags::ShouldUpdate) && ScriptEngine::IsEntityInstantiated(entity))
						{
							ScriptEngine::CallMethod(ScriptEngine::GetEntityInstance(entity.GetUUID()), "OnUpdate");
						}
					}

					myPerformanceTimers.scriptUpdate = timer.ElapsedMillis();
				}

				{
					EPOCH_PROFILE_SCOPE("Scene::OnLateUpdate - C# OnLateUpdate");
					Timer timer;

					auto entities = GetAllEntitiesWith<ScriptComponent>();
					for (auto id : entities)
					{
						Entity entity = Entity(id, this);
						const auto& sc = entities.get<ScriptComponent>(id);
						if (entity.IsActive() && sc.isActive && sc.IsFlagSet(ManagedClassMethodFlags::ShouldLateUpdate) && ScriptEngine::IsEntityInstantiated(entity))
						{
							ScriptEngine::CallMethod(ScriptEngine::GetEntityInstance(entity.GetUUID()), "OnLateUpdate");
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

			// Update UI
			{
				EPOCH_PROFILE_SCOPE("Scene::OnUpdate::UpdateUI");

				if (MouseInViewport())
				{
					auto view = GetAllEntitiesWith<RectComponent, ButtonComponent>();
					for (auto id : view)
					{
						Entity entity = Entity(id, this);
						if (!entity.IsActive()) continue;
				
						const auto& [rc, bc] = view.get<RectComponent, ButtonComponent>(id);
						if (!bc.isActive) continue;
				
						CU::Matrix4x4f transform = entity.GetWorldSpaceTransform().GetMatrix();
				
						const CU::Vector2f size = CU::Vector2f((float)rc.size.x, (float)rc.size.y);
						const CU::Vector2f bl = (CU::Vector2f(0.0f, 0.0f) - rc.pivot) * size;
						const CU::Vector2f tr = (CU::Vector2f(1.0f, 1.0f) - rc.pivot) * size;
						const CU::Vector4f bl4D = transform * CU::Vector4f(bl.x, bl.y, 0.0f, 1.0f);
						const CU::Vector4f tr4D = transform * CU::Vector4f(tr.x, tr.y, 0.0f, 1.0f);

						if (myMousePos.x > bl4D.x && myMousePos.x < tr4D.x &&
							myMousePos.y > bl4D.y && myMousePos.y < tr4D.y)
						{
							if (bc.state == InteractableState::Default)
							{
								bc.state = InteractableState::Hovered;

								if (entity.HasComponent<ScriptComponent>())
								{
									const auto& sc = entity.GetComponent<ScriptComponent>();

									if (ScriptEngine::IsModuleValid(sc.scriptClassHandle) && ScriptEngine::IsEntityInstantiated(entity))
									{
										ScriptEngine::CallMethod(sc.managedInstance, "OnMouseEnter");
									}
								}
							}
							else if (bc.state == InteractableState::Hovered && Input::IsMouseButtonPressed(MouseButton::Left))
							{
								bc.state = InteractableState::Pressed;
							}
							else if (bc.state == InteractableState::Pressed && Input::IsMouseButtonReleased(MouseButton::Left))
							{
								bc.state = InteractableState::Hovered;

								if (entity.HasComponent<ScriptComponent>())
								{
									const auto& sc = entity.GetComponent<ScriptComponent>();

									if (ScriptEngine::IsModuleValid(sc.scriptClassHandle) && ScriptEngine::IsEntityInstantiated(entity))
									{
										ScriptEngine::CallMethod(sc.managedInstance, "OnClick");
									}
								}
							}
						}
						else if (bc.state != InteractableState::Default)
						{
							bc.state = InteractableState::Default;

							if (entity.HasComponent<ScriptComponent>())
							{
								const auto& sc = entity.GetComponent<ScriptComponent>();

								if (ScriptEngine::IsModuleValid(sc.scriptClassHandle) && ScriptEngine::IsEntityInstantiated(entity))
								{
									ScriptEngine::CallMethod(sc.managedInstance, "OnMouseExit");
								}
							}
						}
					}
				}
			}

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

	void Scene::OnRenderGame(std::shared_ptr<SceneRenderer> aRenderer)
	{
		Entity cameraEntity = GetPrimaryCameraEntity();
		if (!cameraEntity) return;
		if (myViewportWidth == 0 || myViewportHeight == 0) return;

		CU::Transform worldTrans = GetWorldSpaceTransform(cameraEntity);
		const CU::Matrix4x4f cameraTransformMatrix = worldTrans.GetMatrix();
		const CU::Matrix4x4f cameraViewMatrix = cameraTransformMatrix.GetFastInverse();
		SceneCamera& camera = cameraEntity.GetComponent<CameraComponent>().camera;
		camera.SetViewportSize(myViewportWidth, myViewportHeight);

		const SceneRendererCamera renderCamera
		(
			(Camera)camera,
			worldTrans.GetTranslation(),
			cameraTransformMatrix,
			cameraViewMatrix,
			camera.GetPerspectiveNearPlane(),
			camera.GetPerspectiveFarPlane(),
			camera.GetPerspectiveFOV(),
			camera.GetAspectRatio()
		);
		Render3DScene(aRenderer, renderCamera, renderCamera, true);
		Render2DScene(aRenderer, renderCamera, true);
	}

	void Scene::OnRenderEditor(std::shared_ptr<SceneRenderer> aRenderer, EditorCamera& aCamera, const EditorRenderSettings& aSettings)
	{
		const SceneRendererCamera renderCamera
		(
			(Camera)aCamera,
			aCamera.GetTransform().GetTranslation(),
			aCamera.GetTransformMatrix(),
			aCamera.GetViewMatrix(),
			aCamera.GetNearPlane(),
			aCamera.GetFarPlane(),
			aCamera.GetFOV(),
			aCamera.GetAspectRatio()
		);
		SceneRendererCamera cullingCamera;


		if (aSettings.cullWithGameCamera)
		{
			Entity cameraEntity = GetPrimaryCameraEntity();
			if (cameraEntity)
			{
				CU::Transform worldTrans = GetWorldSpaceTransform(cameraEntity);
				const CU::Matrix4x4f cameraTransformMatrix = worldTrans.GetMatrix();
				const CU::Matrix4x4f cameraViewMatrix = cameraTransformMatrix.GetFastInverse();
				SceneCamera& camera = cameraEntity.GetComponent<CameraComponent>().camera;
				camera.SetViewportSize(myViewportWidth, myViewportHeight);

				cullingCamera = SceneRendererCamera
				(
					(Camera)camera,
					worldTrans.GetTranslation(),
					cameraTransformMatrix,
					cameraViewMatrix,
					camera.GetPerspectiveNearPlane(),
					camera.GetPerspectiveFarPlane(),
					camera.GetPerspectiveFOV(),
					camera.GetAspectRatio()
				);
			}
			else
			{
				cullingCamera = renderCamera;
			}
		}
		else
		{
			cullingCamera = renderCamera;
		}
		
		Render3DScene(aRenderer, renderCamera, cullingCamera, false, aSettings.postProcessingEnabled);

		if (aSettings.displayUI)
		{
			Render2DScene(aRenderer, renderCamera, false, aSettings.displayUIRects);
		}
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

		if (aEntity.HasComponent<RectComponent>())
		{
			auto trans = aEntity.Transform();
			RectComponent rc = aEntity.GetComponent<RectComponent>();

			CU::Vector3f anchorOffset;
			if (parent && parent.HasComponent<RectComponent>())
			{
				RectComponent prc = parent.GetComponent<RectComponent>();
				const CU::Vector2f size = { (float)prc.size.x, (float)prc.size.y };
				const CU::Vector2f anchorOffset2D = size * rc.anchor - size * prc.pivot;
				anchorOffset = CU::Vector3f(anchorOffset2D);
			}
			else
			{
				anchorOffset = CU::Vector3f(myViewportWidth * rc.anchor.x, myViewportHeight * rc.anchor.y, 0.0f);
			}
			const CU::Vector3f anchoredPos = trans.GetTranslation() * 0.01f + anchorOffset;
			return CU::Transform(anchoredPos, trans.GetRotation(), trans.GetScale()).GetMatrix() * transform;
		}
		else
		{
			return aEntity.TransformMatrix() * transform;
		}
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

	std::unordered_set<AssetHandle> Scene::GetAllSceneReferences()
	{
		std::unordered_set<AssetHandle> sceneList;

		// ScriptComponent
		{
			auto view = myRegistry.view<ScriptComponent>();
			for (auto entity : view)
			{
				const auto& sc = myRegistry.get<ScriptComponent>(entity);
				for (auto fieldID : sc.fieldIDs)
				{
					FieldInfo* fieldInfo = ScriptCache::GetFieldByID(fieldID);
					std::shared_ptr<FieldStorageBase> storage = ScriptEngine::GetFieldStorage(Entity{ entity, this }, fieldID);
					if (!FieldUtils::IsAsset(fieldInfo->type))
					{
						continue;
					}

					if (!fieldInfo->IsArray())
					{
						std::shared_ptr<FieldStorage> fieldStorage = std::static_pointer_cast<FieldStorage>(storage);
						AssetHandle handle = fieldStorage->GetValue<UUID>();
						if (AssetManager::IsMemoryAsset(handle) || Project::GetEditorAssetManager()->GetAssetType(handle) != AssetType::Scene)
						{
							continue;
						}

						if (AssetManager::IsAssetHandleValid(handle))
						{
							sceneList.insert(handle);
						}
					}
					else
					{
						std::shared_ptr<ArrayFieldStorage> arrayFieldStorage = std::static_pointer_cast<ArrayFieldStorage>(storage);

						for (uint32_t i = 0; i < arrayFieldStorage->GetLength(); i++)
						{
							AssetHandle handle = arrayFieldStorage->GetValue<UUID>(i);

							if (AssetManager::IsMemoryAsset(handle))
							{
								continue;
							}

							if (Project::GetEditorAssetManager()->GetAssetType(handle) != AssetType::Scene)
							{
								break;
							}

							if (AssetManager::IsAssetHandleValid(handle))
							{
								sceneList.insert(handle);
							}
						}
					}
				}
			}
		}

		return sceneList;
	}

	std::unordered_set<AssetHandle> Scene::GetAllSceneAssets()
	{
		std::unordered_set<AssetHandle> assetList;

		// MeshRendererComponent
		{
			auto view = myRegistry.view<MeshRendererComponent>();
			for (auto entity : view)
			{
				auto& mrc = myRegistry.get<MeshRendererComponent>(entity);
				if (mrc.mesh)
				{
					if (!AssetManager::IsMemoryAsset(mrc.mesh) && AssetManager::IsAssetHandleValid(mrc.mesh))
					{
						assetList.insert(mrc.mesh);
					}
				}

				if (mrc.materialTable)
				{
					for (size_t i = 0; i < mrc.materialTable->GetMaterialCount(); i++)
					{
						auto materialAssetHandle = mrc.materialTable->GetMaterial((uint32_t)i);

						if (!AssetManager::IsAssetHandleValid(materialAssetHandle))
						{
							continue;
						}

						assetList.insert(materialAssetHandle);

						std::shared_ptr<Material> materialAsset = AssetManager::GetAsset<Material>(materialAssetHandle);

						std::array<AssetHandle, 3> textures =
						{
							materialAsset->GetAlbedoTexture(),
							materialAsset->GetNormalTexture(),
							materialAsset->GetMaterialTexture()
						};

						// Textures
						for (auto texture : textures)
						{
							if (AssetManager::IsAssetHandleValid(texture))
							{
								assetList.insert(texture);
							}
						}
					}
				}
			}
		}

		// SpriteRendererComponent
		{
			auto view = myRegistry.view<SpriteRendererComponent>();
			for (auto entity : view)
			{
				auto& src = myRegistry.get<SpriteRendererComponent>(entity);
				if (src.texture)
				{
					if (AssetManager::IsMemoryAsset(src.texture) || !AssetManager::IsAssetHandleValid(src.texture))
					{
						continue;
					}

					assetList.insert(src.texture);
				}
			}
		}

		// ImageComponent
		{
			auto view = myRegistry.view<ImageComponent>();
			for (auto entity : view)
			{
				auto& ic = myRegistry.get<ImageComponent>(entity);
				if (ic.texture)
				{
					if (AssetManager::IsMemoryAsset(ic.texture) || !AssetManager::IsAssetHandleValid(ic.texture))
					{
						continue;
					}

					assetList.insert(ic.texture);
				}
			}
		}

		// TextRendererComponent
		{
			auto view = myRegistry.view<TextRendererComponent>();
			for (auto entity : view)
			{
				auto& trc = myRegistry.get<TextRendererComponent>(entity);
				if (trc.font)
				{
					if (AssetManager::IsMemoryAsset(trc.font) || !AssetManager::IsAssetHandleValid(trc.font))
					{
						continue;
					}

					assetList.insert(trc.font);
				}
			}
		}

		// ScriptComponent
		{
			auto view = myRegistry.view<ScriptComponent>();
			for (auto entity : view)
			{
				const auto& sc = myRegistry.get<ScriptComponent>(entity);
				for (auto fieldID : sc.fieldIDs)
				{
					FieldInfo* fieldInfo = ScriptCache::GetFieldByID(fieldID);
					std::shared_ptr<FieldStorageBase> storage = ScriptEngine::GetFieldStorage(Entity{ entity, this }, fieldID);
					if (!FieldUtils::IsAsset(fieldInfo->type))
					{
						continue;
					}

					if (!fieldInfo->IsArray())
					{
						std::shared_ptr<FieldStorage> fieldStorage = std::static_pointer_cast<FieldStorage>(storage);
						AssetHandle handle = fieldStorage->GetValue<UUID>();
						if (AssetManager::IsMemoryAsset(handle) || !AssetManager::IsAssetHandleValid(handle))
						{
							continue;
						}

						assetList.insert(handle);
					}
					else
					{
						std::shared_ptr<ArrayFieldStorage> arrayFieldStorage = std::static_pointer_cast<ArrayFieldStorage>(storage);

						for (uint32_t i = 0; i < arrayFieldStorage->GetLength(); i++)
						{
							AssetHandle handle = arrayFieldStorage->GetValue<UUID>(i);

							if (AssetManager::IsMemoryAsset(handle) || !AssetManager::IsAssetHandleValid(handle))
							{
								continue;
							}

							assetList.insert(handle);
						}
					}
				}
			}
		}

		// SkyLightComponent
		{
			auto view = myRegistry.view<SkyLightComponent>();
			for (auto entity : view)
			{
				const auto& slc = myRegistry.get<SkyLightComponent>(entity);
				if (slc.environment)
				{
					if (AssetManager::IsMemoryAsset(slc.environment) || !AssetManager::IsAssetHandleValid(slc.environment))
					{
						continue;
					}

					assetList.insert(slc.environment);
				}
			}
		}

		// SpotlightComponent
		{
			auto view = myRegistry.view<SpotlightComponent>();
			for (auto entity : view)
			{
				const auto& sc = myRegistry.get<SpotlightComponent>(entity);
				if (sc.cookieTexture)
				{
					if (AssetManager::IsMemoryAsset(sc.cookieTexture) || !AssetManager::IsAssetHandleValid(sc.cookieTexture))
					{
						continue;
					}

					assetList.insert(sc.cookieTexture);
				}
			}
		}

		// Collider Components
		{
			// Box
			{
				auto view = myRegistry.view<BoxColliderComponent>();
				for (auto entity : view)
				{
					const auto& cc = myRegistry.get<BoxColliderComponent>(entity);
					if (cc.physicsMaterial)
					{
						if (AssetManager::IsMemoryAsset(cc.physicsMaterial) || !AssetManager::IsAssetHandleValid(cc.physicsMaterial))
						{
							continue;
						}

						assetList.insert(cc.physicsMaterial);
					}
				}
			}

			// Sphere
			{
				auto view = myRegistry.view<SphereColliderComponent>();
				for (auto entity : view)
				{
					const auto& cc = myRegistry.get<SphereColliderComponent>(entity);
					if (cc.physicsMaterial)
					{
						if (AssetManager::IsMemoryAsset(cc.physicsMaterial) || !AssetManager::IsAssetHandleValid(cc.physicsMaterial))
						{
							continue;
						}

						assetList.insert(cc.physicsMaterial);
					}
				}
			}

			// Capsule
			{
				auto view = myRegistry.view<CapsuleColliderComponent>();
				for (auto entity : view)
				{
					const auto& cc = myRegistry.get<CapsuleColliderComponent>(entity);
					if (cc.physicsMaterial)
					{
						if (AssetManager::IsMemoryAsset(cc.physicsMaterial) || !AssetManager::IsAssetHandleValid(cc.physicsMaterial))
						{
							continue;
						}

						assetList.insert(cc.physicsMaterial);
					}
				}
			}
		}

		return assetList;
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

	void Scene::Render3DScene(std::shared_ptr<SceneRenderer> aRenderer, const SceneRendererCamera& aRenderCamera, const SceneRendererCamera& aCullingCamera, bool aIsGameView, bool aWithPostProccessing)
	{
		EPOCH_PROFILE_FUNC();

		std::unordered_map<AssetHandle, std::shared_ptr<Asset>> assetAccelerationMap;

		const Frustum frustum = CreateFrustum(aCullingCamera);

		std::unordered_set<UUID> lastFramesFrustumCulledEntities = myFrustumCulledEntities;
		myFrustumCulledEntities.clear();

		std::unordered_set<UUID> enteredFrustum;
		std::unordered_set<UUID> exitedFrustum;

		// Lighting
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
				myLightEnvironment.pointLights.reserve(pointLights.size());
				for (auto entityID : pointLights)
				{
					Entity entity = Entity(entityID, this);
					if (!entity.IsActive()) continue;

					const auto& plc = pointLights.get<PointLightComponent>(entityID);
					if (!plc.isActive && plc.intensity > 0.0f) continue;

					CU::Transform transform = GetWorldSpaceTransform(entity);

					PointLight& pl = myLightEnvironment.pointLights.emplace_back();
					const auto projMatrix = CU::Matrix4x4f::CreatePerspectiveProjection(90.f * CU::Math::ToRad, 10.f, pl.range, 1.0f);
					pl.viewProjection = transform.GetMatrix().GetFastInverse() * projMatrix;
					pl.position = transform.GetTranslation();
					pl.color = plc.color.GetVector3();
					pl.intensity = plc.intensity;
					pl.range = CU::Math::Max(plc.range, 0.0001f);
					
					auto cookie = AssetManager::GetAsset<Texture2D>(plc.cookieTexture);
					if (!cookie)
					{
						cookie = Renderer::GetWhiteTexture();
					}
					pl.cookie = cookie;
				}

				auto spotlights = GetAllEntitiesWith<SpotlightComponent>();
				myLightEnvironment.spotlights.reserve(spotlights.size());
				for (auto entityID : spotlights)
				{
					Entity entity = Entity(entityID, this);
					if (!entity.IsActive()) continue;

					const auto& slc = spotlights.get<SpotlightComponent>(entityID);
					if (!slc.isActive && slc.intensity > 0.0f) continue;

					CU::Transform transform = GetWorldSpaceTransform(entity);

					Spotlight& sl = myLightEnvironment.spotlights.emplace_back();
					const auto projMatrix = CU::Matrix4x4f::CreatePerspectiveProjection(slc.outerSpotAngle * 2.0f, 10.f, slc.range, 1.0f);
					sl.viewProjection = transform.GetMatrix().GetFastInverse() * projMatrix;
					sl.position = transform.GetTranslation();
					sl.direction = transform.GetForward();
					sl.color = slc.color.GetVector3();
					sl.intensity = slc.intensity;
					sl.range = CU::Math::Max(slc.range, 0.0001f);
					sl.coneAngle = std::cos(slc.outerSpotAngle);
					sl.coneAngleDiff = std::cos(slc.innerSpotAngle) - sl.coneAngle;
					
					auto cookie = AssetManager::GetAsset<Texture2D>(slc.cookieTexture);
					if (!cookie)
					{
						cookie = Renderer::GetWhiteTexture();
					}
					sl.cookie = cookie;
				}
			}
		
		// Post Processing
		{
			EPOCH_PROFILE_SCOPE("Scene::RenderScene::UpdatePostProcessingData");

			myPostProcessingData = PostProcessingData();
			myPostProcessingData.colorGradingLUT = Renderer::GetDefaultColorGradingLut();

			if (aWithPostProccessing)
			{
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
					
					if (vc.colorGrading.enabled)
					{
						myPostProcessingData.bufferData.flags |= (uint32_t)PostProcessingData::Flag::ColorGradingEnabled;

						auto lut = AssetManager::GetAsset<Texture2D>(vc.colorGrading.lut);
						if (!lut)
						{
							lut = Renderer::GetDefaultColorGradingLut();
						}
						myPostProcessingData.colorGradingLUT = lut;
					}
					
					if (vc.vignette.enabled)
					{
						myPostProcessingData.bufferData.flags |= (uint32_t)PostProcessingData::Flag::VignetteEnabled;

						myPostProcessingData.bufferData.vignetteCenter = vc.vignette.center;
						myPostProcessingData.bufferData.vignetteColor = vc.vignette.color;
						myPostProcessingData.bufferData.vignetteIntensity = vc.vignette.intensity;
						myPostProcessingData.bufferData.vignetteSize = vc.vignette.size;
						myPostProcessingData.bufferData.vignetteSmoothness = vc.vignette.smoothness;
					}

					if (vc.distanceFog.enabled)
					{
						myPostProcessingData.bufferData.flags |= (uint32_t)PostProcessingData::Flag::DistanceFogEnabled;

						myPostProcessingData.bufferData.distanceFogColor = vc.distanceFog.color.GetVector3();
						myPostProcessingData.bufferData.distanceFogDensity = vc.distanceFog.density;
						myPostProcessingData.bufferData.distanceFogOffset = vc.distanceFog.offset;
					}

					if (vc.posterization.enabled)
					{
						myPostProcessingData.bufferData.flags |= (uint32_t)PostProcessingData::Flag::PosterizationEnabled;

						myPostProcessingData.bufferData.posterizationSteps = vc.posterization.steps;
					}

					break;
				}
			}
		}

		aRenderer->BeginScene(aRenderCamera);
		
		// Submit Meshes
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
						if (auto it = assetAccelerationMap.find(mrc.mesh); it != assetAccelerationMap.end())
						{
							mesh = std::static_pointer_cast<Mesh>(it->second);
						}
						else
						{
							mesh = AssetManager::GetAssetAsync<Mesh>(mrc.mesh);
							assetAccelerationMap.emplace(mrc.mesh, mesh);
						}

						if (mesh)
						{
							const CU::Matrix4x4f transform = GetWorldSpaceTransformMatrix(entity);

							if (!FrustumIntersection(frustum, mesh->GetBoundingBox().GetGlobal(transform)))
							{
								myFrustumCulledEntities.insert(entity.GetUUID());
								if (!lastFramesFrustumCulledEntities.contains(entity.GetUUID()))
								{
									exitedFrustum.insert(entity.GetUUID());
								}
							}
							else
							{
								aRenderer->SubmitMesh(mesh, mrc.materialTable, transform, (uint32_t)entity);
								if (lastFramesFrustumCulledEntities.contains(entity.GetUUID()))
								{
									enteredFrustum.insert(entity.GetUUID());
								}
							}
							
							if (mrc.castsShadows)
							{
								//TODO: Submit for shadow rendering
							}
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
						if (auto it = assetAccelerationMap.find(smrc.mesh); it != assetAccelerationMap.end())
						{
							mesh = std::static_pointer_cast<Mesh>(it->second);
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
		
		// Submit Sprites
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
					if (auto it = assetAccelerationMap.find(src.texture); it != assetAccelerationMap.end())
					{
						texture = std::static_pointer_cast<Texture2D>(it->second);
					}
					else
					{
						//texture = AssetManager::GetAssetAsync<Texture2D>(src.texture);
						texture = AssetManager::GetAsset<Texture2D>(src.texture); //TODO: Make async
						assetAccelerationMap[src.texture] = texture;
					}

					CU::Matrix4x4f transform = GetWorldSpaceTransformMatrix(entity);
					aRenderer->SubmitQuad(transform, texture, src.tint, src.flipX, src.flipY, (uint32_t)entity);
				}
			}

		// Submit Texts
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
					if (auto it = assetAccelerationMap.find(trc.font); it != assetAccelerationMap.end())
					{
						font = std::static_pointer_cast<Font>(it->second);
					}
					else
					{
						font = AssetManager::GetAssetAsync<Font>(trc.font);
						assetAccelerationMap[trc.font] = font;
					}

					if (!font)
					{
						font = Font::GetDefaultFont();
					}

					SceneRenderer::TextSettings settings;
					settings.maxWidth = trc.maxWidth;
					settings.color = trc.color;
					settings.lineHeightOffset = trc.lineSpacing;
					settings.letterSpacing = trc.letterSpacing;
					settings.billboard = trc.billboard;
					aRenderer->SubmitText(trc.text, font, transform, settings, (uint32_t)entity);
				}
			}

		aRenderer->EndScene();

		if (aIsGameView && myIsPlaying)
			{
				EPOCH_PROFILE_SCOPE("Scene::RenderScene::OnFrustumEnter/Exit");

				for (UUID entityID : enteredFrustum)
				{
					Entity entity = TryGetEntityWithUUID(entityID);
					if (!entity)
					{
						continue;
					}

					if (!entity.HasComponent<ScriptComponent>())
					{
						continue;
					}

					const auto& sc = entity.GetComponent<ScriptComponent>();

					if (!ScriptEngine::IsModuleValid(sc.scriptClassHandle) || !ScriptEngine::IsEntityInstantiated(entity))
					{
						continue;
					}

					ScriptEngine::CallMethod(sc.managedInstance, "OnFrustumEnter");
				}


				for (UUID entityID : exitedFrustum)
				{
					Entity entity = TryGetEntityWithUUID(entityID);
					if (!entity)
					{
						continue;
					}

					if (!entity.HasComponent<ScriptComponent>())
					{
						continue;
					}

					const auto& sc = entity.GetComponent<ScriptComponent>();

					if (!ScriptEngine::IsModuleValid(sc.scriptClassHandle) || !ScriptEngine::IsEntityInstantiated(entity))
					{
						continue;
					}

					ScriptEngine::CallMethod(sc.managedInstance, "OnFrustumExit");
				}
			}
	}

	void Scene::Render2DScene(std::shared_ptr<SceneRenderer> aRenderer, const SceneRendererCamera& aRenderCamera, bool aIsGameView, bool aRenderDebugRects)
	{
		EPOCH_PROFILE_FUNC();

		std::unordered_map<AssetHandle, std::shared_ptr<Asset>> assetAccelerationMap;

		auto screenSpaceRenderer = aRenderer->GetScreenSpaceRenderer();
		if (screenSpaceRenderer)
		{
			if (aIsGameView)
			{
				screenSpaceRenderer->BeginScene(CU::Matrix4x4f::CreateOrthographicProjection(0.0f, (float)myViewportWidth, 0.0f, (float)myViewportHeight), CU::Matrix4x4f::Identity);
			}
			else
			{
				screenSpaceRenderer->BeginScene(aRenderCamera.camera.GetProjectionMatrix(), aRenderCamera.viewMatrix);
			}

			std::map<entt::entity, CU::Color> imageTintMap;
			auto buttonView = GetAllEntitiesWith<ButtonComponent>();
			for (auto id : buttonView)
			{
				Entity entity = Entity(id, this);
				if (!entity.IsActive()) continue;

				const auto& bc = buttonView.get<ButtonComponent>(id);
				if (!bc.isActive) continue;

				switch (bc.state)
				{
				case Epoch::InteractableState::Default:
					imageTintMap.emplace(id, bc.defaultColor);
					break;
				case Epoch::InteractableState::Hovered:
					imageTintMap.emplace(id, bc.hoveredColor);
					break;
				case Epoch::InteractableState::Pressed:
					imageTintMap.emplace(id, bc.pressedColor);
					break;
				}
			}

			// Submit Images
			{
				EPOCH_PROFILE_SCOPE("Scene::RenderScene::SubmitImages");

				auto view = GetAllEntitiesWith<RectComponent, ImageComponent>();
				for (auto id : view)
				{
					Entity entity = Entity(id, this);
					if (!entity.IsActive()) continue;

					const auto& [rc, ic] = view.get<RectComponent, ImageComponent>(id);
					if (!ic.isActive) continue;

					std::shared_ptr<Texture2D> texture;
					if (auto it = assetAccelerationMap.find(ic.texture); it != assetAccelerationMap.end())
					{
						texture = std::static_pointer_cast<Texture2D>(it->second);
					}
					else
					{
						//texture = AssetManager::GetAssetAsync<Texture2D>(src.texture);
						texture = AssetManager::GetAsset<Texture2D>(ic.texture); //TODO: Make async
						assetAccelerationMap[ic.texture] = texture;
					}

					SceneRenderer2D::QuadSetting setting;
					setting.anchor = rc.anchor;
					setting.pivot = rc.pivot;
					setting.flipX = ic.flipX;
					setting.flipY = ic.flipY;

					if (auto it = imageTintMap.find(id); it != imageTintMap.end())
					{
						setting.tint = ic.tint * it->second;
					}
					else
					{
						setting.tint = ic.tint;
					}

					const CU::Matrix4x4f transform = entity.GetWorldSpaceTransform().GetMatrix();
					screenSpaceRenderer->SubmitQuad(transform, rc.size, texture, setting, (uint32_t)entity);

					auto dr = aRenderer->GetDebugRenderer();
					if (dr && aRenderDebugRects)
					{
						const CU::Vector2f size = CU::Vector2f((float)rc.size.x, (float)rc.size.y);
						const CU::Vector2f bl = (CU::Vector2f(0.0f, 0.0f) - rc.pivot) * size;
						const CU::Vector2f br = (CU::Vector2f(1.0f, 0.0f) - rc.pivot) * size;
						const CU::Vector2f tl = (CU::Vector2f(0.0f, 1.0f) - rc.pivot) * size;
						const CU::Vector2f tr = (CU::Vector2f(1.0f, 1.0f) - rc.pivot) * size;
						const CU::Vector4f bl4D = transform * CU::Vector4f(bl.x, bl.y, 0.0f, 1.0f);
						const CU::Vector4f br4D = transform * CU::Vector4f(br.x, br.y, 0.0f, 1.0f);
						const CU::Vector4f tl4D = transform * CU::Vector4f(tl.x, tl.y, 0.0f, 1.0f);
						const CU::Vector4f tr4D = transform * CU::Vector4f(tr.x, tr.y, 0.0f, 1.0f);
						dr->DrawRect(bl4D, br4D, tl4D, tr4D);
					}
				}
			}

			screenSpaceRenderer->EndScene();
		}
	}

	Frustum Scene::CreateFrustum(const SceneRendererCamera& aCamera)
	{
		Frustum frustum;

		const CU::Matrix4x4f viewProj = aCamera.viewMatrix * aCamera.camera.GetProjectionMatrix();

		// Left clipping plane
		frustum.leftPlane =
		{
			viewProj(1, 4) + viewProj(1, 1),
			viewProj(2, 4) + viewProj(2, 1),
			viewProj(3, 4) + viewProj(3, 1),
			viewProj(4, 4) + viewProj(4, 1)
		};

		// Right clipping plane
		frustum.rightPlane =
		{
			viewProj(1, 4) - viewProj(1, 1),
			viewProj(2, 4) - viewProj(2, 1),
			viewProj(3, 4) - viewProj(3, 1),
			viewProj(4, 4) - viewProj(4, 1)
		};

		// Top clipping plane
		frustum.topPlane =
		{
			viewProj(1, 4) - viewProj(1, 2),
			viewProj(2, 4) - viewProj(2, 2),
			viewProj(3, 4) - viewProj(3, 2),
			viewProj(4, 4) - viewProj(4, 2)
		};

		// Bottom clipping plane
		frustum.bottomPlane =
		{
			viewProj(1, 4) + viewProj(1, 2),
			viewProj(2, 4) + viewProj(2, 2),
			viewProj(3, 4) + viewProj(3, 2),
			viewProj(4, 4) + viewProj(4, 2)
		};

		// Near clipping plane
		frustum.nearPlane =
		{
			viewProj(1, 3),
			viewProj(2, 3),
			viewProj(3, 3),
			viewProj(4, 3)
		};

		// Far clipping plane
		frustum.farPlane =
		{
			viewProj(1, 4) - viewProj(1, 3),
			viewProj(2, 4) - viewProj(2, 3),
			viewProj(3, 4) - viewProj(3, 3),
			viewProj(4, 4) - viewProj(4, 3)
		};

		//float verticalFov = 2.0f * std::atanf(std::tanf(aCamera.fov / 2.0f) * aCamera.aspect);
		//const float halfVSide = aCamera.farPlane * std::tanf(verticalFov * 0.5f);
		//const float halfHSide = halfVSide * aCamera.aspect;
		//const CU::Vector3f frontMultFar = aCamera.farPlane * aCamera.viewMatrix.GetForward();
		//
		//frustum.nearPlane = { aCamera.position + aCamera.nearPlane * aCamera.transform.GetForward(), aCamera.transform.GetForward() };
		//frustum.farPlane = { aCamera.position + frontMultFar, -aCamera.transform.GetForward() };
		//frustum.rightPlane = { aCamera.position, CU::Vector3f(frontMultFar - CU::Vector3f(aCamera.transform.GetRight()) * halfHSide).Cross(aCamera.transform.GetUp()) };
		//frustum.leftPlane = { aCamera.position, CU::Vector3f(aCamera.transform.GetUp()).Cross(frontMultFar + CU::Vector3f(aCamera.transform.GetRight()) * halfHSide) };
		//frustum.topPlane = { aCamera.position, CU::Vector3f(aCamera.transform.GetRight()).Cross(frontMultFar - CU::Vector3f(aCamera.transform.GetUp()) * halfVSide) };
		//frustum.bottomPlane = { aCamera.position, CU::Vector3f(frontMultFar + CU::Vector3f(aCamera.transform.GetUp()) * halfVSide).Cross(aCamera.transform.GetRight()) };

		for (Frustum::Plane& plane : frustum.planes)
		{
			const float mag = std::sqrt(plane.normal.x * plane.normal.x + plane.normal.y * plane.normal.y + plane.normal.z * plane.normal.z);
			plane.normal.x /= mag;
			plane.normal.y /= mag;
			plane.normal.z /= mag;
			plane.distance /= mag;
		}

		return frustum;
	}

	bool Scene::FrustumIntersection(const Frustum& aFrustum, const AABB aAABB)
	{
		for (const Frustum::Plane& plane : aFrustum.planes)
		{
			float dist = aAABB.GetCenter().Dot(plane.normal) + plane.distance + aAABB.GetExtents().Length();
			if (dist < 0)
			{
				return false;
			}
		}

		return true;
	}
	
	std::pair<float, float> Scene::GetMouseViewportSpace() const
	{
		return { (myMousePos.x / myViewportWidth) * 2.0f - 1.0f, (myMousePos.y / myViewportHeight) * 2.0f - 1.0f };
	}
	
	bool Scene::MouseInViewport()
	{
		auto [mouseX, mouseY] = GetMouseViewportSpace();
		return (mouseX > -1.0f && mouseX < 1.0f && mouseY > -1.0f && mouseY < 1.0f);
	}
}
