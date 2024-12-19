#include "epch.h"
#include "PhysicsBody.h"

namespace Epoch
{
	PhysicsBody::PhysicsBody(Entity aEntity) : myEntity(aEntity)
	{
		if (aEntity.HasComponent<RigidbodyComponent>())
		{
			myIsStatic = false;
			myMass = aEntity.GetComponent<RigidbodyComponent>().mass;
		}
	}

	void PhysicsBody::CreateCollisionShapesForEntity(Entity aEntity, bool aIsFirst)
	{
		bool isCompound = false;
		if (aIsFirst && aEntity.HasComponent<RigidbodyComponent>() && !aEntity.HasAny<BoxColliderComponent, SphereColliderComponent, CapsuleColliderComponent>())
		{
			isCompound = true;
		}

		if (isCompound)
		{
			for (Entity child : aEntity.GetChildren())
			{
				if (!child.HasAny<BoxColliderComponent, SphereColliderComponent, CapsuleColliderComponent>())
				{
					continue;
				}

				CreateCollisionShapesForEntity(child, false);
			}
		}
		else
		{
			if (aEntity.HasComponent<BoxColliderComponent>())
			{
				myShapes[Physics::ShapeType::Box].push_back(BoxShape::Create(aEntity, myMass, !aIsFirst));
			}

			if (aEntity.HasComponent<SphereColliderComponent>())
			{
				myShapes[Physics::ShapeType::Sphere].push_back(SphereShape::Create(aEntity, myMass, !aIsFirst));
			}

			if (aEntity.HasComponent<CapsuleColliderComponent>())
			{
				myShapes[Physics::ShapeType::Capsule].push_back(CapsuleShape::Create(aEntity, myMass, !aIsFirst));
			}
		}
	}
}