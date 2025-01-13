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

		myEventCallback = std::make_unique<PhysXEventCallback>([this](PhysicsEventType aEventType, Entity aEntityA, Entity aEntityB)
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

	bool PhysXScene::Raycast(CU::Vector3f aOrigin, CU::Vector3f aDirection, float aMaxDistance, HitInfo* outHit)
	{
		physx::PxRaycastBuffer hitInfo;

		physx::PxQueryFilterData filterData;
		filterData.flags |= physx::PxQueryFlag::Enum::ePREFILTER;
		filterData.flags |= physx::PxQueryFlag::Enum::eDYNAMIC;
		filterData.flags |= physx::PxQueryFlag::Enum::eSTATIC;
		filterData.data.word0 = BIT(0);

		bool hit = myScene->raycast(PhysXUtils::ToPhysXVector(aOrigin), PhysXUtils::ToPhysXVector(aDirection), aMaxDistance, hitInfo, physx::PxHitFlag::ePOSITION | physx::PxHitFlag::eNORMAL, filterData);

		if (hit && outHit)
		{
			outHit->entity = *(uint64_t*)hitInfo.block.actor->userData;
			outHit->position = PhysXUtils::FromPhysXVector(hitInfo.block.position);
			outHit->normal = PhysXUtils::FromPhysXVector(hitInfo.block.normal);
			outHit->distance = hitInfo.block.distance;
		}
		return hit;
	}

	bool PhysXScene::ShapeCast(const ShapeCastInfo* aShapeCastInfo, HitInfo* outHit)
	{
		physx::PxSweepHit hitInfo;
		physx::PxTransform initialPose = physx::PxTransform(PhysXUtils::ToPhysXVector(aShapeCastInfo->origin));
		physx::PxVec3 sweepDirection = PhysXUtils::ToPhysXVector(aShapeCastInfo->direction);

		bool hit = false;
		switch (aShapeCastInfo->GetShapeType())
		{
		case Physics::ShapeType::Box:
		{
			const auto* boxCastInfo = reinterpret_cast<const BoxCastInfo*>(aShapeCastInfo);
			physx::PxBoxGeometry shape = physx::PxBoxGeometry(boxCastInfo->halfExtent);
			hit = physx::PxSceneQueryExt::sweepSingle(*myScene, shape, initialPose, sweepDirection, aShapeCastInfo->maxDistance, physx::PxHitFlag::ePOSITION | physx::PxHitFlag::eNORMAL | physx::PxHitFlag::eMTD, hitInfo);
			break;
		}
		case Physics::ShapeType::Sphere:
		{
			const auto* sphereCastInfo = reinterpret_cast<const SphereCastInfo*>(aShapeCastInfo);
			physx::PxSphereGeometry shape = physx::PxSphereGeometry(sphereCastInfo->radius);
			hit = physx::PxSceneQueryExt::sweepSingle(*myScene, shape, initialPose, sweepDirection, aShapeCastInfo->maxDistance, physx::PxHitFlag::ePOSITION | physx::PxHitFlag::eNORMAL | physx::PxHitFlag::eMTD, hitInfo);
			break;
		}
		default:
			EPOCH_ASSERT(false, "Cannot cast mesh shapes!");
			break;
		}

		if (!hit)
		{
			return false;
		}

		if (hit && outHit)
		{
			outHit->entity = *(uint64_t*)hitInfo.actor->userData;
			outHit->position = PhysXUtils::FromPhysXVector(hitInfo.position);
			outHit->normal = PhysXUtils::FromPhysXVector(hitInfo.normal);
			outHit->distance = hitInfo.distance;
		}

		return hit;
	}

	std::vector<UUID> PhysXScene::OverlapShape(const ShapeOverlapInfo* aShapeOverlapInfo)
	{
		physx::PxTransform shapePose = physx::PxTransform(PhysXUtils::ToPhysXVector(aShapeOverlapInfo->origin));

		int numberOfHits = 0;
		std::unique_ptr<physx::PxOverlapHit[]> hitOv = std::make_unique<physx::PxOverlapHit[]>(4096);
		switch (aShapeOverlapInfo->GetShapeType())
		{
		case Physics::ShapeType::Box:
		{
			const auto* boxOverlapInfo = reinterpret_cast<const BoxOverlapInfo*>(aShapeOverlapInfo);
			physx::PxBoxGeometry shape = physx::PxBoxGeometry(boxOverlapInfo->halfExtent);
			numberOfHits = physx::PxSceneQueryExt::overlapMultiple(*myScene, shape, shapePose, hitOv.get(), 4096);
			break;
		}
		case Physics::ShapeType::Sphere:
		{
			const auto* sphereOverlapInfo = reinterpret_cast<const SphereOverlapInfo*>(aShapeOverlapInfo);
			physx::PxSphereGeometry shape = physx::PxSphereGeometry(sphereOverlapInfo->radius);
			numberOfHits = physx::PxSceneQueryExt::overlapMultiple(*myScene, shape, shapePose, hitOv.get(), 4096);
			break;
		}
		default:
			EPOCH_ASSERT(false, "Cannot cast mesh shapes!");
			break;
		}

		std::vector<UUID> overlapHitBuffer;
		for (size_t i = 0; i < numberOfHits; i++)
		{
			const physx::PxOverlapHit& hit = hitOv[i];
			UUID entityID = *(UUID*)hit.actor->userData;
			overlapHitBuffer.push_back(entityID);
		}

		return overlapHitBuffer;
	}

	void PhysXScene::Teleport(Entity aEntity, const CU::Vector3f& aTargetPosition, const CU::Quatf& aTargetRotation)
	{
		auto body = std::static_pointer_cast<PhysXBody>(GetPhysicsBody(aEntity));
		((physx::PxRigidActor*)body->myActor)->setGlobalPose({ PhysXUtils::ToPhysXVector(aTargetPosition), PhysXUtils::ToPhysXQuat(aTargetRotation) });
	}
}
