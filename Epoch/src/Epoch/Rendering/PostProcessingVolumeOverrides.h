#pragma once
#include "Epoch/Assets/Asset.h"
#include <CommonUtilities/Color.h>
#include <CommonUtilities/Math/Vector/Vector3.hpp>
#include <CommonUtilities/Math/Vector/Vector2.hpp>

namespace Epoch::PostProcessing
{
	struct ColorGrading
	{
		bool enabled = false;

		AssetHandle lut = 0;
	};

	struct Vignette
	{
		bool enabled = false;

		struct Data
		{
			CU::Vector3f color = CU::Color::Black.GetVector3();
			float size = 1.0f;
			CU::Vector2f center = { 0.5f, 0.5f };
			float intensity = 1.0f;
			float smoothness = 1.0f;
		} data;
	};
}
