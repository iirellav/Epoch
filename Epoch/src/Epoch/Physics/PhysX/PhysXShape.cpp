#include "epch.h"
#include "PhysXShape.h"
#include "PhysXAPI.h"
#include "PhysXUtils.h"
#include "Epoch/Physics/PhysicsSystem.h"
#include "Epoch/Physics/PhysicsLayer.h"

namespace Epoch
{
	PhysXBoxShape::PhysXBoxShape(Entity aEntity, float aMass, bool aIsCompoundShape)
	{
		const auto& colliderComp = aEntity.GetComponent<BoxColliderComponent>();
		const auto& layer = PhysicsLayerManager::GetLayer(colliderComp.layerID);

		PhysXAPI* api = (PhysXAPI*)PhysicsSystem::GetAPI();
		physx::PxFilterData filterData;
		filterData.word0 = layer.bitValue;		// word0 = own ID
		filterData.word1 = layer.collidesWith;  // word1 = ID mask to filter pairs that trigger a contact callback;

		CU::Vector3f offset;
		CU::Quatf rotationalOffset;

		const CU::Transform worldTransform = aEntity.GetWorldSpaceTransform();

		myExtents = colliderComp.halfSize * worldTransform.GetScale();

		if (aIsCompoundShape)
		{
			const CU::Transform& trans = aEntity.Transform();
			offset = trans.GetTranslation() + colliderComp.offset;
			rotationalOffset = trans.GetRotationQuat();
		}
		else
		{
			offset = colliderComp.offset;
		}

		myShape = api->GetPhysicsSystem()->createShape(physx::PxBoxGeometry(myExtents), *api->GetMaterial(colliderComp.physicsMaterial));
		myShape->setLocalPose({ PhysXUtils::ToPhysXVector(offset), PhysXUtils::ToPhysXQuat(rotationalOffset) });

		myShape->setSimulationFilterData(filterData);
		myShape->setQueryFilterData(filterData);

		if (colliderComp.isTrigger)
		{
			myShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
			myShape->setFlag(physx::PxShapeFlag::Enum::eTRIGGER_SHAPE, true);
		}
	}

	PhysXBoxShape::~PhysXBoxShape()
	{
		myShape->release();
		myShape = nullptr;
	}

	PhysXSphereShape::PhysXSphereShape(Entity aEntity, float aMass, bool aIsCompoundShape)
	{
		const auto& colliderComp = aEntity.GetComponent<SphereColliderComponent>();
		const auto& layer = PhysicsLayerManager::GetLayer(colliderComp.layerID);

		PhysXAPI* api = (PhysXAPI*)PhysicsSystem::GetAPI();

		physx::PxFilterData filterData;
		filterData.word0 = layer.bitValue;		// word0 = own ID
		filterData.word1 = layer.collidesWith;  // word1 = ID mask to filter pairs that trigger a contact callback;

		CU::Vector3f offset;
		CU::Quatf rotationalOffset;

		const CU::Transform worldTransform = aEntity.GetWorldSpaceTransform();

		const CU::Vector3f& scale = worldTransform.GetScale();
		myRadius = colliderComp.radius * CU::Math::Max(CU::Math::Max(scale.x, scale.y), scale.z);

		if (aIsCompoundShape)
		{
			const CU::Transform& trans = aEntity.Transform();
			offset = trans.GetTranslation() + colliderComp.offset;
			rotationalOffset = trans.GetRotationQuat();
		}
		else
		{
			offset = colliderComp.offset;
		}

		myShape = api->GetPhysicsSystem()->createShape(physx::PxSphereGeometry(myRadius), *api->GetMaterial(colliderComp.physicsMaterial));
		myShape->setLocalPose({ PhysXUtils::ToPhysXVector(offset), PhysXUtils::ToPhysXQuat(rotationalOffset) });

		myShape->setSimulationFilterData(filterData);
		myShape->setQueryFilterData(filterData);

		if (colliderComp.isTrigger)
		{
			myShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
			myShape->setFlag(physx::PxShapeFlag::Enum::eTRIGGER_SHAPE, true);
		}
	}

	PhysXSphereShape::~PhysXSphereShape()
	{
		myShape->release();
		myShape = nullptr;
	}

	PhysXCapsuleShape::PhysXCapsuleShape(Entity aEntity, float aMass, bool aIsCompoundShape)
	{
		const auto& colliderComp = aEntity.GetComponent<CapsuleColliderComponent>();
		const auto& layer = PhysicsLayerManager::GetLayer(colliderComp.layerID);

		PhysXAPI* api = (PhysXAPI*)PhysicsSystem::GetAPI();

		physx::PxFilterData filterData;
		filterData.word0 = layer.bitValue;		// word0 = own ID
		filterData.word1 = layer.collidesWith;  // word1 = ID mask to filter pairs that trigger a contact callback;

		CU::Vector3f offset;
		CU::Quatf rotationalOffset;

		const CU::Transform worldTransform = aEntity.GetWorldSpaceTransform();

		const CU::Vector3f& scale = worldTransform.GetScale();
		myRadius = colliderComp.radius * CU::Math::Max(scale.x, scale.z);
		myHeight = (colliderComp.height * scale.y) - myRadius * 2.0f;

		if (aIsCompoundShape)
		{
			const CU::Transform& trans = aEntity.Transform();
			offset = trans.GetTranslation() + colliderComp.offset;
			rotationalOffset = trans.GetRotationQuat();
		}
		else
		{
			offset = colliderComp.offset;
		}

		rotationalOffset *= CU::Quatf(CU::Vector3f(0.0f, 0.0f, 90.0f) * CU::Math::ToRad);

		myShape = api->GetPhysicsSystem()->createShape(physx::PxCapsuleGeometry(myRadius, myHeight * 0.5f), *api->GetMaterial(colliderComp.physicsMaterial));
		myShape->setLocalPose({ PhysXUtils::ToPhysXVector(offset), PhysXUtils::ToPhysXQuat(rotationalOffset) });

		myShape->setSimulationFilterData(filterData);
		myShape->setQueryFilterData(filterData);

		if (colliderComp.isTrigger)
		{
			myShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
			myShape->setFlag(physx::PxShapeFlag::Enum::eTRIGGER_SHAPE, true);
		}
	}

	PhysXCapsuleShape::~PhysXCapsuleShape()
	{
		myShape->release();
		myShape = nullptr;
	}
}
