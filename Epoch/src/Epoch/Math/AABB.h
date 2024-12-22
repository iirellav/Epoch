#pragma once
#include <CommonUtilities/Math/Vector/Vector3.hpp>

namespace Epoch
{
	struct AABB
	{
		CU::Vector3f min, max;

		AABB() = default;
		AABB(const CU::Vector3f& aMix, const CU::Vector3f& aMax) : min(aMix), max(aMax) {}
		AABB(const CU::Vector3f& aCenter, float aI, float aJ, float aK)
		{
			min = aCenter - CU::Vector3f(aI, aJ, aK);
			max = aCenter + CU::Vector3f(aI, aJ, aK);
		}

		CU::Vector3f GetCenter() const { return 0.5f * (min + max); }
		CU::Vector3f GetExtents() const { return 0.5f * (max - min); }

		AABB GetGlobal(const CU::Matrix4x4f& aTransform) const;
	};
}
