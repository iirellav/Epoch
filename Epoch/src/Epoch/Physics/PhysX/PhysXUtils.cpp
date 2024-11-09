#include "epch.h"
#include "PhysXUtils.h"

namespace Epoch::PhysXUtils
{
	physx::PxVec3 ToPhysXVector(const CU::Vector3f& aVector)
	{
		return *(physx::PxVec3*)&aVector;
	}

	physx::PxQuat ToPhysXQuat(const CU::Quatf& aQuat)
	{
		return physx::PxQuat(aQuat.x, aQuat.y, aQuat.z, aQuat.w);
	}

	CU::Vector3f FromPhysXVector(const physx::PxVec3& aVector)
	{
		return *(CU::Vector3f*)&aVector;
	}

	CU::Quatf FromPhysXQuat(const physx::PxQuat& aQuat)
	{
		return CU::Quatf(aQuat.w, aQuat.x, aQuat.y, aQuat.z);
	}
}