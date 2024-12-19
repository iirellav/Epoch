#pragma once
#include <memory>
#include <CommonUtilities/Math/Vector/Vector3.hpp>
#include "Epoch/Debug/Log.h"
#include "Epoch/Scene/Entity.h"
#include "Epoch/Physics/PhysicsTypes.h"

namespace Epoch
{
	namespace ShapeUtils
	{
		inline const char* ShapeTypeToString(Physics::ShapeType aType)
		{
			switch (aType)
			{
			case Physics::ShapeType::Box:		return "Box";
			case Physics::ShapeType::Sphere:		return "Sphere";
			case Physics::ShapeType::Capsule:	return "Capsule";
			}

			EPOCH_ASSERT(false, "Unknown shape!");
			return "";
		}
	}

	class PhysicsShape
	{
	public:
		virtual ~PhysicsShape() = default;

		virtual void* GetNativeShape() const = 0;

	protected:
		PhysicsShape(Physics::ShapeType aType) : myType(aType) {}

	private:
		Physics::ShapeType myType;
	};

	class BoxShape : public PhysicsShape
	{
	public:
		BoxShape() : PhysicsShape(Physics::ShapeType::Box) {}
		virtual ~BoxShape() = default;

		virtual CU::Vector3f GetHalfSize() const = 0;

		static std::shared_ptr<BoxShape> Create(Entity aEntity, float aMass, bool aIsCompoundShape = false);
	};

	class SphereShape : public PhysicsShape
	{
	public:
		SphereShape() : PhysicsShape(Physics::ShapeType::Sphere) {}
		virtual ~SphereShape() = default;

		virtual float GetRadius() const = 0;

		static std::shared_ptr<SphereShape> Create(Entity aEntity, float aMass, bool aIsCompoundShape = false);
	};

	class CapsuleShape : public PhysicsShape
	{
	public:
		CapsuleShape() : PhysicsShape(Physics::ShapeType::Capsule) {}
		virtual ~CapsuleShape() = default;

		virtual float GetRadius() const = 0;
		virtual float GetHeight() const = 0;

		static std::shared_ptr<CapsuleShape> Create(Entity aEntity, float aMass, bool aIsCompoundShape = false);
	};
}
