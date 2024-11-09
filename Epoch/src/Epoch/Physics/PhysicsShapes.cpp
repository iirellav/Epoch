#include "epch.h"
#include "PhysicsShapes.h"
#include "PhysicsAPI.h"
#include "PhysX/PhysXShape.h"

namespace Epoch
{
	std::shared_ptr<BoxShape> BoxShape::Create(Entity aEntity, float aMass, bool aIsCompoundShape)
	{
		switch (PhysicsAPI::Current())
		{
		case PhysicsAPIType::PhysX: return std::make_shared<PhysXBoxShape>(aEntity, aMass, aIsCompoundShape);
		}
		return nullptr;
	}

	std::shared_ptr<SphereShape> SphereShape::Create(Entity aEntity, float aMass, bool aIsCompoundShape)
	{
		switch (PhysicsAPI::Current())
		{
		case PhysicsAPIType::PhysX: return std::make_shared<PhysXSphereShape>(aEntity, aMass, aIsCompoundShape);
		}
		return nullptr;
	}

	std::shared_ptr<CapsuleShape> CapsuleShape::Create(Entity aEntity, float aMass, bool aIsCompoundShape)
	{
		switch (PhysicsAPI::Current())
		{
		case PhysicsAPIType::PhysX: return std::make_shared<PhysXCapsuleShape>(aEntity, aMass, aIsCompoundShape);
		}
		return nullptr;
	}
}
