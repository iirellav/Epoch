#pragma once
#include "Epoch/Physics/PhysicsEventCallback.h"
#include <PxPhysics.h>
#include <PxPhysicsAPI.h>

namespace Epoch
{
	class PhysXEventCallback : public physx::PxSimulationEventCallback
	{
	public:
		PhysXEventCallback(const PhysicsEventCallbackFn& aEventCallback) : 
			myEventCallback(aEventCallback), physx::PxSimulationEventCallback() { }

		void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override { }

		void onWake(physx::PxActor** actors, physx::PxU32 count) override { }

		void onSleep(physx::PxActor** actors, physx::PxU32 count) override { }

		void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;

		void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;

		void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override { }
		
	private:
		void OnCollisionEnter(UUID aBody1, UUID aBody2);
		void OnCollisionExit(UUID aBody1, UUID aBody2);
		void OnTriggerEnter(UUID aBody1, UUID aBody2);
		void OnTriggerExit(UUID aBody1, UUID aBody2);

	private:
		PhysicsEventCallbackFn myEventCallback;
	};
}
