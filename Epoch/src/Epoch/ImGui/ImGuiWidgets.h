#pragma once
#include <array>
#include <CommonUtilities/Gradient.h>
#include "Epoch/Assets/Asset.h"

namespace Epoch
{
	class Scene;
}

namespace Epoch::UI::Widgets
{
	void GradientBar(const CU::Gradient* aGradient, CU::Vector2f aBarPos, float aMaxWidth, float aHeight);

	void CubicBezier(std::array<CU::Vector2f, 4> aPoints, CU::Vector2f aBarPos, float aMaxWidth, float aHeight, bool aDrawLines = false, float aThickness = 2.0f);

	void Spinner(const char* aLabel, float aRadius, float aThickness, uint32_t aColor);
	void BufferingBar(const char* aLabel, float aValue, CU::Vector2f aSize, uint32_t aBgCol, uint32_t aFgCol);

	bool AssetSearchPopup(const char* aPopupID, AssetType aAssetType, AssetHandle& outSelected, bool* outClear);
	bool EntitySearchPopup(const char* aPopupID, std::shared_ptr<Scene> aScene, UUID& outSelected, bool* outClear);
}
