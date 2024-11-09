#pragma once
#include <CommonUtilities/Math/Vector/Vector3.hpp>

namespace Epoch
{
	struct PhysicsSettings
	{
		float fixedTimestep = 1.0f / 60.0f;
		CU::Vector3f gravity = { 0.0f, -982.0f, 0.0f };
	};
}
