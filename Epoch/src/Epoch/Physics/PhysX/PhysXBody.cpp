#include "epch.h"
#include "PhysXBody.h"
#include "PhysXUtils.h"
#include "Epoch/Physics/PhysicsSystem.h"
#include "Epoch/Physics/PhysicsTypes.h"
#include "PhysXAPI.h"
#include "PhysXUtils.h"
#include <PxPhysics.h>
#include <PxRigidStatic.h>
#include <PxRigidDynamic.h>

namespace Epoch
{
	PhysXBody::PhysXBody(Entity aEntity) : PhysicsBody(aEntity)
	{
		CreateCollisionShapesForEntity(aEntity);
		if (myShapes.empty()) return;
		
		PhysXAPI* api = (PhysXAPI*)PhysicsSystem::GetAPI();

		const CU::Transform worldTransform = myEntity.GetWorldSpaceTransform();
		const physx::PxTransform pxTrans = physx::PxTransform(PhysXUtils::ToPhysXVector(worldTransform.GetTranslation()), PhysXUtils::ToPhysXQuat(worldTransform.GetRotationQuat()));

		if (myIsStatic)
		{
			physx::PxRigidStatic* pxBody = api->GetPhysicsSystem()->createRigidStatic(pxTrans);
			for (const auto& [shapeType, shapes] : myShapes)
			{
				for (const auto& shape : shapes)
				{
					pxBody->attachShape(*(physx::PxShape*)shape->GetNativeShape());
				}
			}
			
			myActor = (physx::PxActor*)pxBody;
		}
		else
		{
			const auto& rigidbodyComponent = myEntity.GetComponent<RigidbodyComponent>();

			physx::PxRigidDynamic* pxBody = api->GetPhysicsSystem()->createRigidDynamic(pxTrans);
			for (const auto& [shapeType, shapes] : myShapes)
			{
				for (const auto& shape : shapes)
				{
					pxBody->attachShape(*(physx::PxShape*)shape->GetNativeShape());
				}
			}

			pxBody->setLinearDamping(rigidbodyComponent.linearDrag);
			pxBody->setAngularDamping(rigidbodyComponent.angularDrag);
			physx::PxRigidBodyExt::setMassAndUpdateInertia(*pxBody, rigidbodyComponent.mass);

			const CU::Vector3f initialLinearVelocity = 
				worldTransform.GetForward() * rigidbodyComponent.initialLinearVelocity.z + 
				worldTransform.GetRight() * rigidbodyComponent.initialLinearVelocity.x + 
				worldTransform.GetUp() * rigidbodyComponent.initialLinearVelocity.y;

			const CU::Matrix3x3f rotationMatrix = CU::Quatf(worldTransform.GetRotation()).GetRotationMatrix3x3();
			const CU::Vector3f initialAngularVelocity = rotationMatrix * rigidbodyComponent.initialAngularVelocity;

			pxBody->setAngularVelocity(PhysXUtils::ToPhysXVector(initialAngularVelocity));
			pxBody->setLinearVelocity(PhysXUtils::ToPhysXVector(initialLinearVelocity));

			pxBody->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, !rigidbodyComponent.useGravity);

			physx::PxRigidDynamicLockFlags lockFlags = (physx::PxRigidDynamicLockFlags)(uint8_t)rigidbodyComponent.constraints;
			pxBody->setRigidDynamicLockFlags(lockFlags);

			myActor = (physx::PxActor*)pxBody;
		}

		myActor->setName(myEntity.GetName().c_str());
		myActor->userData = new UUID(myEntity.GetUUID());
	}

	PhysXBody::~PhysXBody()
	{
		if (myActor)
		{
			myActor->release();
			myActor = nullptr;
		}
	}

	float PhysXBody::GetMass()
	{
		return ((physx::PxRigidDynamic*)myActor)->getMass();
	}

	void PhysXBody::SetMass(float aMass)
	{
		physx::PxRigidBodyExt::setMassAndUpdateInertia(*(physx::PxRigidDynamic*)myActor, aMass);
	}

	bool PhysXBody::GetUseGravity()
	{
		return ((physx::PxRigidDynamic*)myActor)->getActorFlags() & physx::PxActorFlag::eDISABLE_GRAVITY;
	}

	void PhysXBody::SetUseGravity(bool aState)
	{
		((physx::PxRigidDynamic*)myActor)->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, aState);
	}

	float PhysXBody::GetDrag()
	{
		return ((physx::PxRigidDynamic*)myActor)->getLinearDamping();
	}

	void PhysXBody::SetDrag(float aDrag)
	{
		((physx::PxRigidDynamic*)myActor)->setLinearDamping(aDrag);
	}

	float PhysXBody::GetAngularDrag()
	{
		return ((physx::PxRigidDynamic*)myActor)->getAngularDamping();
	}

	void PhysXBody::SetAngularDrag(float aDrag)
	{
		((physx::PxRigidDynamic*)myActor)->setAngularDamping(aDrag);
	}

	CU::Vector3f PhysXBody::GetVelocity()
	{
		return PhysXUtils::FromPhysXVector(((physx::PxRigidDynamic*)myActor)->getLinearVelocity());
	}

	void PhysXBody::SetVelocity(CU::Vector3f aVelocity)
	{
		((physx::PxRigidDynamic*)myActor)->setLinearVelocity(PhysXUtils::ToPhysXVector(aVelocity));
	}

	CU::Vector3f PhysXBody::GetAngularVelocity()
	{
		return PhysXUtils::FromPhysXVector(((physx::PxRigidDynamic*)myActor)->getAngularVelocity());
	}

	void PhysXBody::SetAngularVelocity(CU::Vector3f aVelocity)
	{
		((physx::PxRigidDynamic*)myActor)->setAngularVelocity(PhysXUtils::ToPhysXVector(aVelocity));
	}

	CU::Vector3f PhysXBody::GetPosition()
	{
		return PhysXUtils::FromPhysXVector(((physx::PxRigidDynamic*)myActor)->getGlobalPose().p);
	}

	void PhysXBody::SetPosition(CU::Vector3f aPosition)
	{
		physx::PxRigidDynamic* rb = ((physx::PxRigidDynamic*)myActor);
		const auto pos = PhysXUtils::ToPhysXVector(aPosition);
		const auto rot = rb->getGlobalPose().q;

		rb->setGlobalPose({ pos, rot });
	}

	CU::Vector3f PhysXBody::GetRotation()
	{
		return PhysXUtils::FromPhysXQuat(((physx::PxRigidDynamic*)myActor)->getGlobalPose().q).GetEulerAngles();
	}

	void PhysXBody::SetRotation(CU::Vector3f aRotation)
	{
		physx::PxRigidDynamic* rb = ((physx::PxRigidDynamic*)myActor);
		const auto pos = rb->getGlobalPose().p;
		const auto rot = PhysXUtils::ToPhysXQuat(CU::Quatf(aRotation));

		rb->setGlobalPose({ pos, rot });
	}

	PhysicsAxis PhysXBody::GetConstraints()
	{
		physx::PxRigidDynamic* rb = ((physx::PxRigidDynamic*)myActor);
		return (PhysicsAxis)(uint8_t)rb->getRigidDynamicLockFlags();
	}

	void PhysXBody::SetConstraints(PhysicsAxis aConstraints)
	{
		physx::PxRigidDynamicLockFlags lockFlags = (physx::PxRigidDynamicLockFlags)(uint8_t)aConstraints;
		physx::PxRigidDynamic* rb = ((physx::PxRigidDynamic*)myActor);
		rb->setRigidDynamicLockFlags(lockFlags);
	}

	void PhysXBody::AddForce(CU::Vector3f aForce, ForceMode aForceMode)
	{
		((physx::PxRigidBody*)myActor)->addForce(PhysXUtils::ToPhysXVector(aForce), (physx::PxForceMode::Enum)aForceMode);
	}
	
	void PhysXBody::AddForceAtPosition(CU::Vector3f aForce, CU::Vector3f aPosition, ForceMode aForceMode)
	{
		physx::PxRigidBodyExt::addForceAtPos(*(physx::PxRigidDynamic*)myActor, PhysXUtils::ToPhysXVector(aForce), PhysXUtils::ToPhysXVector(aPosition), (physx::PxForceMode::Enum)aForceMode);
	}

	void PhysXBody::AddTorque(CU::Vector3f aTorque, ForceMode aForceMode)
	{
		((physx::PxRigidBody*)myActor)->addTorque(PhysXUtils::ToPhysXVector(aTorque), (physx::PxForceMode::Enum)aForceMode);
	}

	void PhysXBody::AddRadialImpulse(CU::Vector3f aOrigin, float aRadius, float aStrength)
	{
		auto rigidbody = ((physx::PxRigidBody*)myActor);

		//CU::Vector3f centerOfMassPosition = PhysXUtils::FromPhysXVector(rigidbody->getCMassLocalPose().p);
		CU::Vector3f centerOfMassPosition = myEntity.GetWorldSpaceTransform().GetTranslation();
		
		CU::Vector3f direction = centerOfMassPosition - aOrigin;
		float distance = direction.Length();
		direction.Normalize();

		if (distance > aRadius) return;
		
		CU::Vector3f impulse = direction * aStrength;
		
		((physx::PxRigidBody*)myActor)->addForce(impulse);
	}
}