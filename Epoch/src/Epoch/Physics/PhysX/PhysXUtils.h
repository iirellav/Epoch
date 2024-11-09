#pragma once
#include <CommonUtilities/Math/Vector/Vector3.hpp>
#include <CommonUtilities/Math/Quaternion.hpp>
#include <foundation/PxVec3.h>
#include <foundation/PxQuat.h>

namespace Epoch::PhysXUtils
{
	physx::PxVec3 ToPhysXVector(const CU::Vector3f& aVector);
	physx::PxQuat ToPhysXQuat(const CU::Quatf& aQuat);

	CU::Vector3f FromPhysXVector(const physx::PxVec3& aVector);
	CU::Quatf FromPhysXQuat(const physx::PxQuat& aQuat);
}
