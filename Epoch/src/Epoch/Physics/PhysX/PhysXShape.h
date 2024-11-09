#pragma once
#include "Epoch/Physics/PhysicsShapes.h"
#include <PxPhysicsAPI.h>

namespace Epoch
{
	class PhysXBoxShape : public BoxShape
	{
	public:
		PhysXBoxShape(Entity aEntity, float aMass, bool aIsCompoundShape);
		~PhysXBoxShape() override;

		void* GetNativeShape() const override { return (void*)myShape; }

		CU::Vector3f GetHalfSize() const override { return myExtents; }
		
	private:
		physx::PxShape* myShape = nullptr;

		CU::Vector3f myExtents = CU::Vector3f::One * 50.0f;
	};
	
	class PhysXSphereShape : public SphereShape
	{
	public:
		PhysXSphereShape(Entity aEntity, float aMass, bool aIsCompoundShape);
		~PhysXSphereShape() override;
		
		void* GetNativeShape() const override { return (void*)myShape; }

		float GetRadius() const override { return myRadius; }
		
	private:
		physx::PxShape* myShape = nullptr;

		float myRadius = 50.0f;
	};
	
	class PhysXCapsuleShape : public CapsuleShape
	{
	public:
		PhysXCapsuleShape(Entity aEntity, float aMass, bool aIsCompoundShape);
		~PhysXCapsuleShape() override;
		
		void* GetNativeShape() const override { return (void*)myShape; }
		
		float GetRadius() const override { return myRadius; }
		float GetHeight() const override { return myHeight; }
		
	private:
		physx::PxShape* myShape = nullptr;

		float myRadius = 50.0f;
		float myHeight = 200.0f;
	};
}
