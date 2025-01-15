#include "epch.h"
#include "PhysXCharacterController.h"
#include "PhysXAPI.h"
#include "PhysXUtils.h"
#include "Epoch/Physics/PhysicsSystem.h"
#include <PxPhysicsAPI.h>
#include <PxShape.h>

namespace Epoch
{
	PhysXCharacterController::PhysXCharacterController(Entity aEntity)
	{
		PhysXAPI* api = (PhysXAPI*)PhysicsSystem::GetAPI();

		const CU::Transform worldTransform = aEntity.GetWorldSpaceTransform();
		const auto& ccComponent = aEntity.GetComponent<CharacterControllerComponent>();
		const auto& layer = PhysicsLayerManager::GetLayer(ccComponent.layerID);
		myLayerValue = layer.bitValue;
		myCollisionValue = layer.collidesWith;

		physx::PxCapsuleControllerDesc desc;
		desc.height = ccComponent.height - ccComponent.radius * 2.0f;
		desc.radius = ccComponent.radius;
		desc.slopeLimit = std::cos(ccComponent.slopeLimit * CU::Math::ToRad);
		desc.stepOffset = ccComponent.stepOffset;
		desc.climbingMode = physx::PxCapsuleClimbingMode::eCONSTRAINED;
		CU::Vector3f offsetPos = worldTransform.GetTranslation() + ccComponent.offset;
		desc.position = { offsetPos.x, offsetPos.y, offsetPos.z };
		desc.material = api->GetDefaultMaterial();
		desc.contactOffset = 0.01f;

		if (!desc.isValid())
		{
			LOG_ERROR_TAG("Physics", "Invalid character controller desc");
		}

		myCharacterController = api->GetControllerManager()->createController(desc);
		EPOCH_ASSERT(myCharacterController, "Failed to create character controller!");
		if (myCharacterController)
		{
			auto* actor = myCharacterController->getActor();
			physx::PxShape* shapes;
			actor->userData = new UUID(aEntity.GetUUID());
			actor->getShapes(&shapes, 1);
			if (shapes)
			{
				physx::PxFilterData filterData;
				filterData.word0 = myLayerValue;		// word0 = own ID
				filterData.word1 = myCollisionValue;	// word1 = ID mask to filter pairs that trigger a contact callback;

				shapes->setSimulationFilterData(filterData);
				shapes->setQueryFilterData(filterData);
			}
		}
	}

	PhysXCharacterController::~PhysXCharacterController()
	{
		myCharacterController = nullptr;
	}

	float PhysXCharacterController::GetStepOffset()
	{
		return myCharacterController->getStepOffset();
	}

	void PhysXCharacterController::SetStepOffset(float aValue)
	{
		myCharacterController->setStepOffset(aValue);
	}

	float PhysXCharacterController::GetSlopeLimit()
	{
		return myCharacterController->getSlopeLimit();
	}

	void PhysXCharacterController::SetSlopeLimit(float aValue)
	{
		myCharacterController->setSlopeLimit(aValue);
	}

	void PhysXCharacterController::Resize(float aHeight)
	{
		myCharacterController->resize(aHeight);
	}

	void PhysXCharacterController::Move(const CU::Vector3f& aDisplacement)
	{
		myDisplacement += aDisplacement;
	}

	void PhysXCharacterController::Simulate(float aTimeStep)
	{
		physx::PxFilterData filterData;
		filterData.word0 = myLayerValue;		// word0 = own ID
		filterData.word1 = myCollisionValue;	// word1 = ID mask to filter pairs that trigger a contact callback;

		physx::PxControllerFilters ccFilterData;
		ccFilterData.mFilterFlags = physx::PxQueryFlag::Enum::eDYNAMIC | physx::PxQueryFlag::Enum::eSTATIC | physx::PxQueryFlag::Enum::ePREFILTER;
		ccFilterData.mFilterData = &filterData;

		myCharacterController->move(PhysXUtils::ToPhysXVector(myDisplacement), 0.1f, aTimeStep, ccFilterData);
		myDisplacement = CU::Vector3f::Zero;
	}
}
