#include "epch.h"
#include "PhysicsScene.h"
#include <CommonUtilities/Timer.h>
#include "Epoch/Scene/Scene.h"
#include "Epoch/Script/ScriptEngine.h"

namespace Epoch
{
	PhysicsScene::PhysicsScene(Scene* aScene) : mySceneContext(aScene)
	{
	}

	PhysicsScene::~PhysicsScene()
	{
		mySceneContext = nullptr;
	}

	std::shared_ptr<PhysicsBody> PhysicsScene::GetPhysicsBodyWithID(UUID aID)
	{
		if (auto iter = myDynamicPhysicsBodies.find(aID); iter != myDynamicPhysicsBodies.end())
		{
			return iter->second;
		}

		return nullptr;
	}

	std::shared_ptr<PhysicsBody> PhysicsScene::GetPhysicsBody(Entity aEntity)
	{
		return GetPhysicsBodyWithID(aEntity.GetUUID());
	}

	std::shared_ptr<CharacterController> PhysicsScene::GetCharacterControllerWithID(UUID aID)
	{
		if (auto iter = myCharacterControllers.find(aID); iter != myCharacterControllers.end())
		{
			return iter->second;
		}

		return nullptr;
	}

	std::shared_ptr<CharacterController> PhysicsScene::GetCharacterController(Entity aEntity)
	{
		return GetCharacterControllerWithID(aEntity.GetUUID());
	}

	void PhysicsScene::CreatePhysicsBodies()
	{
		EPOCH_PROFILE_FUNC();
		
		std::vector<UUID> compoundedEntities;
		std::vector<UUID> compoundedChildEntities;
		std::vector<UUID> other;

		auto view = mySceneContext->GetAllEntitiesWith<TransformComponent>();
		for (auto enttID : view)
		{
			Entity entity = { enttID, mySceneContext };

			if (!entity.IsActive()) continue;

			bool isCompound = false;
			if (entity.HasComponent<RigidbodyComponent>() && !entity.HasAny<BoxColliderComponent, SphereColliderComponent, CapsuleColliderComponent>())
			{
				isCompound = true;
				compoundedEntities.push_back(entity.GetUUID());

				for (UUID child : entity.Children())
				{
					Entity childEntity = mySceneContext->GetEntityWithUUID(child);
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
			Entity entity = mySceneContext->GetEntityWithUUID(entityID);
			CreateBody(entity);
		}

		for (UUID entityID : other)
		{
			if (std::find(compoundedChildEntities.begin(), compoundedChildEntities.end(), entityID) != compoundedChildEntities.end())
			{
				continue;
			}

			Entity entity = mySceneContext->GetEntityWithUUID(entityID);
			CreateBody(entity);
		}
	}

	void PhysicsScene::CreateCharacterControllers()
	{
		EPOCH_PROFILE_FUNC();

		auto view = mySceneContext->GetAllEntitiesWith<CharacterControllerComponent>();
		for (auto enttID : view)
		{
			Entity entity = { enttID, mySceneContext };

			if (!entity.IsActive())
			{
				continue;
			}

			CreateCharacterController(entity);
		}
	}

	void PhysicsScene::PreSimulate()
	{
		EPOCH_PROFILE_FUNC();

		{
			ScriptEngine::UpdateFixedDeltaTime(PhysicsSystem::GetSettings().fixedTimestep);

			EPOCH_PROFILE_SCOPE("Scene::OnFixedUpdate - C# OnUpdate");
			Timer timer;

			for (const auto& [entityID, entityInstance] : ScriptEngine::GetEntityInstances())
			{
				Entity entity = mySceneContext->TryGetEntityWithUUID(entityID);
				if (!entity)
				{
					continue;
				}

				if (entity.IsActive() && ScriptEngine::IsEntityInstantiated(entity))
				{
					//TODO: Only call if entity has a rigidbody(?)
					ScriptEngine::CallMethod(entityInstance, "OnFixedUpdate");
				}
			}

			mySceneContext->GetPerformanceTimers().scriptFixedUpdate = timer.ElapsedMillis();
		}
	}

	static void CallScriptPhysicsCallback(const char* aMethodName, Entity aEntityA, Entity aEntityB)
	{
		if (!aEntityA.HasComponent<ScriptComponent>())
		{
			return;
		}

		const auto& sc = aEntityA.GetComponent<ScriptComponent>();

		if (!ScriptEngine::IsModuleValid(sc.scriptClassHandle) || !ScriptEngine::IsEntityInstantiated(aEntityA))
		{
			return;
		}

		ScriptEngine::CallMethod(sc.managedInstance, aMethodName, aEntityB.GetUUID());
	}

	void PhysicsScene::PostSimulate()
	{
		EPOCH_PROFILE_FUNC();

		for (const auto& event : myPhysicsEvents)
		{
			Entity entityA = mySceneContext->TryGetEntityWithUUID(event.entityA);
			Entity entityB = mySceneContext->TryGetEntityWithUUID(event.entityB);

			if (!entityA || !entityB)
			{
				continue;
			}

			switch (event.type)
			{
				case PhysicsEventType::CollisionEnter:
				{
					CallScriptPhysicsCallback("OnCollisionEnterInternal", entityA, entityB);
					CallScriptPhysicsCallback("OnCollisionEnterInternal", entityB, entityA);
					break;
				}
				case PhysicsEventType::CollisionExit:
				{
					CallScriptPhysicsCallback("OnCollisionExitInternal", entityA, entityB);
					CallScriptPhysicsCallback("OnCollisionExitInternal", entityB, entityA);
					break;
				}
				case PhysicsEventType::TriggerEnter:
				{
					CallScriptPhysicsCallback("OnTriggerEnterInternal", entityA, entityB);
					CallScriptPhysicsCallback("OnTriggerEnterInternal", entityB, entityA);
					break;
				}
				case PhysicsEventType::TriggerExit:
				{
					CallScriptPhysicsCallback("OnTriggerExitInternal", entityA, entityB);
					CallScriptPhysicsCallback("OnTriggerExitInternal", entityB, entityA);
					break;
				}
			}
		}

		myPhysicsEvents.clear();
	}

	void PhysicsScene::OnPhysicsEvent(PhysicsEventType aType, Entity aEntityA, Entity aEntityB)
	{
		auto& contactEvent = myPhysicsEvents.emplace_back();;
		contactEvent.type = aType;
		contactEvent.entityA = aEntityA.GetUUID();
		contactEvent.entityB = aEntityB.GetUUID();
	}

	void PhysicsScene::SubStepStrategy()
	{
		EPOCH_PROFILE_FUNC();

		float deltaTime = CU::Timer::GetDeltaTime() * mySceneContext->GetTimeScale();
		deltaTime = CU::Math::Min(deltaTime, 0.0333f);

		if (myAccumulator > PhysicsSystem::GetSettings().fixedTimestep)
		{
			myAccumulator = 0.0f;
		}

		myAccumulator += deltaTime;
		if (myAccumulator < PhysicsSystem::GetSettings().fixedTimestep)
		{
			mySubSteps = 0;
			return;
		}

		mySubSteps = (uint32_t)(myAccumulator / PhysicsSystem::GetSettings().fixedTimestep);
		myAccumulator -= (float)mySubSteps * PhysicsSystem::GetSettings().fixedTimestep;
	}
}
