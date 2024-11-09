#pragma once
#include <cstdint>
#include <CommonUtilities/Math/Vector/Vector3.hpp>

namespace Epoch
{
	struct HitInfo
	{
		uint64_t entity = 0;
		CU::Vector3f position = CU::Vector3f::Zero;
		CU::Vector3f normal = CU::Vector3f::Zero;
		float distance = 0.0f;
	};
}
