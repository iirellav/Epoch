#include "epch.h"
#include "PhysXEventCallback.h"
#include "Epoch/Script/ScriptEngine.h"

namespace Epoch
{
	void PhysXEventCallback::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
	{
		//TODO:: Loop through pairs?

		if (pairs->events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
		{
			OnCollisionEnter(*(UUID*)pairHeader.actors[0]->userData, *(UUID*)pairHeader.actors[1]->userData);
		}
		else if (pairs->events & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
		{
			OnCollisionExit(*(UUID*)pairHeader.actors[0]->userData, *(UUID*)pairHeader.actors[1]->userData);
		}
	}

	void PhysXEventCallback::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
	{
		//TODO:: Loop through pairs?

		if (pairs->status == physx::PxPairFlag::Enum::eNOTIFY_TOUCH_FOUND)
		{
			OnTriggerEnter(*(UUID*)pairs->otherActor->userData, *(UUID*)pairs->triggerActor->userData);
		}
		else
		{
			OnTriggerExit(*(UUID*)pairs->otherActor->userData, *(UUID*)pairs->triggerActor->userData);
		}
	}

	void PhysXEventCallback::OnCollisionEnter(UUID aBody1, UUID aBody2)
	{
		Entity entity1 = ScriptEngine::GetSceneContext()->TryGetEntityWithUUID(aBody1);
		Entity entity2 = ScriptEngine::GetSceneContext()->TryGetEntityWithUUID(aBody2);

		if (!entity1 || !entity2)
		{
			return;
		}

		myEventCallback(EventType::CollisionEnter, entity1, entity2);
	}

	void PhysXEventCallback::OnCollisionExit(UUID aBody1, UUID aBody2)
	{
		Entity entity1 = ScriptEngine::GetSceneContext()->TryGetEntityWithUUID(aBody1);
		Entity entity2 = ScriptEngine::GetSceneContext()->TryGetEntityWithUUID(aBody2);

		if (!entity1 || !entity2)
		{
			return;
		}

		myEventCallback(EventType::CollisionExit, entity1, entity2);
	}

	void PhysXEventCallback::OnTriggerEnter(UUID aBody1, UUID aBody2)
	{
		Entity entity1 = ScriptEngine::GetSceneContext()->TryGetEntityWithUUID(aBody1);
		Entity entity2 = ScriptEngine::GetSceneContext()->TryGetEntityWithUUID(aBody2);

		if (!entity1 || !entity2)
		{
			return;
		}

		myEventCallback(EventType::TriggerEnter, entity1, entity2);
	}

	void PhysXEventCallback::OnTriggerExit(UUID aBody1, UUID aBody2)
	{
		Entity entity1 = ScriptEngine::GetSceneContext()->TryGetEntityWithUUID(aBody1);
		Entity entity2 = ScriptEngine::GetSceneContext()->TryGetEntityWithUUID(aBody2);

		if (!entity1 || !entity2)
		{
			return;
		}

		myEventCallback(EventType::TriggerExit, entity1, entity2);
	}
}
