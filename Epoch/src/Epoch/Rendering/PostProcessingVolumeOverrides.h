#pragma once
#include "Epoch/Assets/Asset.h"
#include <CommonUtilities/Color.h>
#include <CommonUtilities/Math/Vector/Vector3.hpp>
#include <CommonUtilities/Math/Vector/Vector2.hpp>

namespace Epoch::PostProcessing
{
	enum class Tonemap : uint32_t
	{
		None,
		Unreal,
		Lottes,
		ACES
	};

	struct Tonemapping
	{
		bool enabled = false;

		Tonemap tonemap = Tonemap::Unreal;
	};

	struct ColorGrading
	{
		bool enabled = false;

		AssetHandle lut = 0;
	};

	struct Vignette
	{
		bool enabled = false;

		CU::Vector3f color = CU::Color::Black.GetVector3();
		float size = 1.0f;
		CU::Vector2f center = { 0.5f, 0.5f };
		float intensity = 1.0f;
		float smoothness = 1.0f;
	};

	struct DistanceFog
	{
		bool enabled = false;
		
		CU::Color color = CU::Color::White;
		float density = 0.3f;
		float offset = 0.0f;
	};
}
