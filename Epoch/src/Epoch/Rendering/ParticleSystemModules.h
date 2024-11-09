#pragma once
#include <vector>
#include <CommonUtilities/Math/Vector/Vector.h>
#include <CommonUtilities/Gradient.h>

namespace Epoch::ParticleSystem
{
	struct BurstData
	{
		float time = 0.0f;
		unsigned count = 10;
		float probability = 1.0f;
	};

	struct Emission
	{
		float rate = 10.0f; // Particles per sec.
		std::vector<BurstData> bursts;
	};

	enum class EmitterShape { Sphere, Box, Cone };
	struct Shape
	{
		EmitterShape shape = EmitterShape::Sphere;

		float angle = 20.0f; // Cone only
		float radius = 50.0f; // Sphere & Cone only

		CU::Vector3f position = CU::Vector3f::Zero;
		CU::Vector3f rotation = CU::Vector3f::Zero;
		CU::Vector3f scale = CU::Vector3f::One;
	};

	struct VelocityOverLifetime
	{
		CU::Vector3f linear = CU::Vector3f::Zero;
	};

	struct ColorOverLifetime
	{
		CU::Gradient colorGradient;
	};

	struct SizeOverLifetime
	{
		float start = 1.0f;
		float end = 1.0f;
	};
}
