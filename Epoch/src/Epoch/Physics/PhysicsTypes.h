#pragma once
#include <magic_enum.hpp>
using namespace magic_enum::bitwise_operators; // out-of-the-box bitwise operators for enums.

namespace Epoch
{
	enum class ForceMode : uint8_t
	{
		Force,
		Impulse,
		VelocityChange,
		Acceleration
	};

	enum class PhysicsAxis : uint8_t
	{
		None = 0,

		TranslationX = (1 << 0),
		TranslationY = (1 << 1),
		TranslationZ = (1 << 2),
		Translation = TranslationX | TranslationY | TranslationZ,

		RotationX = (1 << 3),
		RotationY = (1 << 4),
		RotationZ = (1 << 5),
		Rotation = RotationX | RotationY | RotationZ
		
	};
}
