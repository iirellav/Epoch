#include "epch.h"
#include "AABB.h"

namespace Epoch
{
	AABB AABB::GetGlobal(const CU::Matrix4x4f& aTransform) const
	{
		const CU::Vector3f globalCenter(aTransform * CU::Vector4f(GetCenter(), 1.f));

		const CU::Vector3f extents = GetExtents() * aTransform.GetScale();
		const CU::Vector3f right = aTransform.GetRight() * extents.x;
		const CU::Vector3f up = aTransform.GetUp() * extents.y;
		const CU::Vector3f forward = aTransform.GetForward() * extents.z;

		const float newIi =
			std::abs(CU::Vector3f::Right.Dot(right)) +
			std::abs(CU::Vector3f::Right.Dot(up)) +
			std::abs(CU::Vector3f::Right.Dot(forward));

		const float newIj =
			std::abs(CU::Vector3f::Up.Dot(right)) +
			std::abs(CU::Vector3f::Up.Dot(up)) +
			std::abs(CU::Vector3f::Up.Dot(forward));

		const float newIk =
			std::abs(CU::Vector3f::Forward.Dot(right)) +
			std::abs(CU::Vector3f::Forward.Dot(up)) +
			std::abs(CU::Vector3f::Forward.Dot(forward));

		return AABB(globalCenter, newIi, newIj, newIk);
	}
}
