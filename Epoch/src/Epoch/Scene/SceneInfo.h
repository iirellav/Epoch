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
		struct BufferData
		{
			CU::Vector3f vignetteColor = CU::Color::Black.GetVector3();
			float vignetteSize = 1.0f;

			CU::Vector2f vignetteCenter = { 0.5f, 0.5f };
			float vignetteIntensity = 1.0f;
			float vignetteSmoothness = 1.0f;

			PostProcessing::Tonemap tonemap;
			uint32_t flags = 0; //colorGradingEnabled, vignetteEnabled, distanceFogEnabled, posterizationEnabled
			float distanceFogDensity = 0.0f;
			float distanceFogOffset = 0.0f;

			CU::Color distanceFogColor;
		} bufferData;
	};
}
