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

		std::shared_ptr<Texture2D> cookie;
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

		std::shared_ptr<Texture2D> cookie;
	};

	struct LightEnvironment
	{
		std::weak_ptr<Environment> environment;
		float environmentIntensity = 1.0f;

		DirectionalLight directionalLight;
		std::vector<PointLight> pointLights;
		std::vector<Spotlight> spotlights;
	};

	struct PostProcessingData
	{
		enum class Flag : uint32_t
		{
			None = 0,
			ColorGradingEnabled		= BIT(0),
			VignetteEnabled			= BIT(1),
			DistanceFogEnabled		= BIT(2),
			PosterizationEnabled	= BIT(3),
		};

		AssetHandle colorGradingLUT = 0;
		struct BufferData
		{
			CU::Vector3f vignetteColor = CU::Color::Black.GetVector3();
			float vignetteSize = 1.0f;

			CU::Vector2f vignetteCenter = { 0.5f, 0.5f };
			float vignetteIntensity = 1.0f;
			float vignetteSmoothness = 1.0f;

			PostProcessing::Tonemap tonemap = PostProcessing::Tonemap::None;
			uint32_t flags = 0; //colorGradingEnabled, vignetteEnabled, distanceFogEnabled, posterizationEnabled
			float distanceFogDensity = 0.0f;
			float distanceFogOffset = 0.0f;

			CU::Vector3f distanceFogColor;
			uint32_t posterizationSteps = 0;
		} bufferData;
	};
}
