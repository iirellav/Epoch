#pragma once
#include <vector>
#include <memory>
#include <CommonUtilities/Math/Vector/Vector3.hpp>
#include <CommonUtilities/Math/Matrix/Matrix4x4.hpp>
#include "Epoch/Assets/Asset.h"
#include "Epoch/Rendering/PostProcessingVolumeOverrides.h"

namespace Epoch
{
	class Environment;

	struct DirectionalLight
	{
		CU::Vector3f direction;
		float intensity = 0.0f;

		CU::Vector3f color;
		float padding = 0.0f;
	};

	struct PointLight
	{
		CU::Matrix4x4f viewProjection;

		CU::Vector3f position;
		float intensity = 0.0f;

		CU::Vector3f color;
		float range = 0.0f;

		AssetHandle cookie;
	};

	struct Spotlight
	{
		CU::Matrix4x4f viewProjection;

		CU::Vector3f position;
		float intensity = 0.0f;

		CU::Vector3f direction;
		float range = 0.0f;

		CU::Vector3f color;
		float coneAngle = 0.0f;

		float coneAngleDiff = 0.0f;
		CU::Vector3f padding;

		AssetHandle cookie;
	};

	struct LightEnvironment
	{
		std::weak_ptr<Environment> environment;
		float environmentIntensity = 0.0f;

		DirectionalLight directionalLight;
		std::vector<PointLight> pointLights;
		std::vector<Spotlight> spotlights;
	};

	struct PostProcessingData
	{
		AssetHandle colorGradingLUT = 0;

		PostProcessing::Vignette::Data vignetteData;
	};
}
