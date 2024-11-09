#include "epch.h"
#include "PhysXScene.h"
#include "Epoch/Scene/Scene.h"
#include "Epoch/Core/Application.h"
#include "Epoch/Physics/PhysicsSystem.h"
#include "PhysXAPI.h"
#include "PhysXUtils.h"
#include "PhysXBody.h"
#include "PhysXFilterShader.h"
#include "PhysXCharacterController.h"
#include <geometry/PxGeometry.h>

namespace Epoch
{
	PhysXScene::PhysXScene(Scene* aScene) : PhysicsScene(aScene)
	{
		EPOCH_PROFILE_FUNC();

		PhysXAPI* api = (PhysXAPI*)PhysicsSystem::GetAPI();

		myEventCallback = std::make_unique<PhysXEventCallback>([this](EventType aEventType, Entity aEntityA, Entity aEntityB)
			{
				OnPhysicsEvent(aEventType, aEntityA, aEntityB);
			});

		physx::PxSceneDesc sceneDesc(api->GetPhysicsSystem()->getTolerancesScale());
		sceneDesc.gravity = PhysXUtils::ToPhysXVector(PhysicsSystem::GetSettings().gravity);
		sceneDesc.cpuDispatcher = api->GetDispatcher();
		sceneDesc.filterShader = contactReportFilterShader;
		sceneDesc.simulationEventCallback = myEventCallback.get();

		myScene = api->GetPhysicsSystem()->createScene(sceneDesc);

		//myScene->setSimulationEventCallback(myEventCallback.get());

		api->ConnectPVD();
		api->InitControllerManager(this);

		CreatePhysicsBodies();
		CreateCharacterControllers();
	}

	PhysXScene::~PhysXScene()
	{
	}

	void PhysXScene::Destroy()
	{
		PhysXAPI* api = (PhysXAPI*)PhysicsSystem::GetAPI();
		api->DisconnectPVD();
		api->ShutdownControllerManager();
	}

	void PhysXScene::Simulate()
	{
		EPOCH_PROFILE_FUNC();

		SubStepStrategy();

		PreSimulate();

		bool sync = false;
		for (size_t i = 0; i < mySubSteps; i++)
		{
			for (auto& [id, pxController] : myCharacterControllers)
			{
				pxController->Simulate(PhysicsSystem::GetSettings().fixedTimestep);
			}

			{
				EPOCH_PROFILE_SCOPE("PhysXSystem::Update");
				myScene->simulate(PhysicsSystem::GetSettings().fixedTimestep);
				sync |= myScene->fetchResults(true);
			}

			PostSimulate();
		}

		if (sync)
		{
			EPOCH_PROFILE_SCOPE("PhysXScene::SynchronizeTransform");

			for (auto& [id, pxBody] : myDynamicPhysicsBodies)
			{
				Entity entity = mySceneContext->TryGetEntityWithUUID(id);
				if (!entity)
				{
					continue;
				}

				physx::PxTransform pxTrans = ((physx::PxRigidDynamic*)(((PhysXBody*)pxBody.get())->myActor))->getGlobalPose();
				
				auto& transComp = entity.GetComponent<TransformComponent>();
				const CU::Vector3f orgScale = entity.Transform().GetScale();
				transComp.transform.SetTranslation(PhysXUtils::FromPhysXVector(pxTrans.p));
				transComp.transform.SetRotation(PhysXUtils::FromPhysXQuat(pxTrans.q).GetEulerAngles());

				mySceneContext->ConvertToLocalSpace(entity);
				transComp.transform.SetScale(orgScale);
			}

			for (auto& [id, pxController] : myCharacterControllers)
			{
				Entity entity = mySceneContext->TryGetEntityWithUUID(id);
				if (!entity)
				{
					continue;
				}

				const auto& ccComponent = entity.GetComponent<CharacterControllerComponent>();
				const physx::PxExtendedVec3 position = ((PhysXCharacterController*)pxController.get())->GetCharacterController()->getPosition();
				auto& transComp = entity.GetComponent<TransformComponent>();
				const CU::Vector3f newPos = CU::Vector3f((float)position.x, (float)position.y, (float)position.z) - ccComponent.offset;
				transComp.transform.SetTranslation(newPos);
			}
		}
	}

	std::shared_ptr<PhysicsBody> Epoch::PhysXScene::CreateBody(Entity aEntity)
	{
		if (!aEntity.HasAny<RigidbodyComponent, BoxColliderComponent, SphereColliderComponent, CapsuleColliderComponent>())
		{
			return nullptr;
		}

		auto physicsBody = std::make_shared<PhysXBody>(aEntity);
		if (!physicsBody->IsValid())
		{
			return nullptr;
		}

		if (physicsBody->IsStatic())
		{
			myStaticPhysicsBodies[aEntity.GetUUID()] = physicsBody;
		}
		else
		{
			myDynamicPhysicsBodies[aEntity.GetUUID()] = physicsBody;
		}

		if (!myScene->addActor(*physicsBody->myActor))
		{
			LOG_WARNING("Failed to add physics actor to scene.");
		}
		return physicsBody;
	}

	void PhysXScene::DestroyBody(Entity aEntity)
	{
		if (auto it = myStaticPhysicsBodies.find(aEntity.GetUUID()); it != myStaticPhysicsBodies.end())
		{
			myStaticPhysicsBodies.erase(it);
		}
		else if (auto it = myDynamicPhysicsBodies.find(aEntity.GetUUID()); it != myDynamicPhysicsBodies.end())
		{
			myDynamicPhysicsBodies.erase(it);
		}
	}

	std::shared_ptr<CharacterController> Epoch::PhysXScene::CreateCharacterController(Entity aEntity)
	{
		if (!aEntity.HasComponent<CharacterControllerComponent>())
		{
			return nullptr;
		}

		auto characterController = std::make_shared<PhysXCharacterController>(aEntity);

		if (characterController->GetCharacterController())
		{
			myCharacterControllers[aEntity.GetUUID()] = characterController;
			return characterController;
		}
		else
		{
			return nullptr;
		}
	}

	CU::Vector3f PhysXScene::GetGravity() const
	{
		return PhysXUtils::FromPhysXVector(myScene->getGravity());
	}

	void PhysXScene::SetGravity(const CU::Vector3f& aGravity)
	{
		myScene->setGravity(PhysXUtils::ToPhysXVector(aGravity));
	}

	bool PhysXScene::Raycast(CU::Vector3f aOrigin, CU::Vector3f aDirection, float aMaxDistance, HitInfo* outHitInfo)
	{
		physx::PxRaycastBuffer hitInfo;
		bool hit = myScene->raycast(PhysXUtils::ToPhysXVector(aOrigin), PhysXUtils::ToPhysXVector(aDirection), aMaxDistance, hitInfo);
		if (hit)
		{
			outHitInfo->entity = *(uint64_t*)hitInfo.block.actor->userData;
			outHitInfo->position = PhysXUtils::FromPhysXVector(hitInfo.block.position);
			outHitInfo->normal = PhysXUtils::FromPhysXVector(hitInfo.block.normal);
			outHitInfo->distance = hitInfo.block.distance;
		}
		return hit;
	}

	void PhysXScene::AddRadialImpulse(CU::Vector3f aOrigin, float aRadius, float aStrength)
	{
		physx::PxSphereGeometry overlapShape = physx::PxSphereGeometry(aRadius);
		physx::PxTransform shapePose = physx::PxTransform(PhysXUtils::ToPhysXVector(aOrigin));

		std::unique_ptr<physx::PxOverlapHit[]> hitOv = std::make_unique<physx::PxOverlapHit[]>(4096);
		int numberOfHits = physx::PxSceneQueryExt::overlapMultiple(*myScene, overlapShape, shapePose, hitOv.get(), 4096);
		//int numberOfHits = physx::PxSceneQueryExt::overlapMultiple(*myScene, overlapShape, shapePose, hitOv.get(), 4096, physx::PxQueryFilterData(physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eNO_BLOCK));

		for (size_t i = 0; i < numberOfHits; i++)
		{
			const auto& hit = hitOv[i];
			UUID entityID = *(UUID*)hit.actor->userData;

			auto entityBody = GetPhysicsBodyWithID(entityID);

			if (!entityBody)
			{
				continue;
			}

			entityBody->AddRadialImpulse(aOrigin, aRadius, aStrength);
		}
	}

	void PhysXScene::Teleport(Entity aEntity, const CU::Vector3f& aTargetPosition, const CU::Quatf& aTargetRotation)
	{
		auto body = std::static_pointer_cast<PhysXBody>(GetPhysicsBody(aEntity));
		((physx::PxRigidActor*)body->myActor)->setGlobalPose({ PhysXUtils::ToPhysXVector(aTargetPosition), PhysXUtils::ToPhysXQuat(aTargetRotation) });
	}
}
