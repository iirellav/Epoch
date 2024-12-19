#pragma once
#include <cstdint>
#include <CommonUtilities/Math/Vector/Vector3.hpp>
#include "Epoch/Physics/PhysicsTypes.h"

namespace Epoch
{
	struct HitInfo
	{
		uint64_t entity = 0;
		CU::Vector3f position = CU::Vector3f::Zero;
		CU::Vector3f normal = CU::Vector3f::Zero;
		float distance = 0.0f;
	};


	struct ShapeCastInfo
	{
		CU::Vector3f origin;
		CU::Vector3f direction;
		float maxDistance = 0.0f;

		Physics::ShapeType GetShapeType() const { return myShape; }

	protected:
		ShapeCastInfo(Physics::ShapeType aShape) : myShape(aShape) {}

	private:
		Physics::ShapeType myShape;
	};

	struct BoxCastInfo : public ShapeCastInfo
	{
		BoxCastInfo() : ShapeCastInfo(Physics::ShapeType::Box) {}

		CU::Vector3f halfExtent;
	};

	struct SphereCastInfo : public ShapeCastInfo
	{
		SphereCastInfo() : ShapeCastInfo(Physics::ShapeType::Sphere) {}

		float radius = 0.0f;
	};


	struct ShapeOverlapInfo
	{
		CU::Vector3f origin;

		Physics::ShapeType GetShapeType() const { return myShape; }

	protected:
		ShapeOverlapInfo(Physics::ShapeType aShape) : myShape(aShape) {}

	private:
		Physics::ShapeType myShape;
	};

	struct BoxOverlapInfo : public ShapeOverlapInfo
	{
		BoxOverlapInfo() : ShapeOverlapInfo(Physics::ShapeType::Box) {}

		CU::Vector3f halfExtent;
	};

	struct SphereOverlapInfo : public ShapeOverlapInfo
	{
		SphereOverlapInfo() : ShapeOverlapInfo(Physics::ShapeType::Sphere) {}

		float radius = 0.0f;
	};
}
