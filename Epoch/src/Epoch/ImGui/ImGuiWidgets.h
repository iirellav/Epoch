#pragma once
#include <CommonUtilities/Gradient.h>
#include "Epoch/Assets/Asset.h"

namespace Epoch
{
	class Scene;
}

namespace Epoch::UI::Widgets
{
	void GradientBar(const CU::Gradient* aGradient, CU::Vector2f aBarPos, float aMaxWidth, float aHeight);

	bool AssetSearchPopup(const char* aPopupID, AssetType aAssetType, AssetHandle& outSelected, bool* outClear);
	bool EntitySearchPopup(const char* aPopupID, std::shared_ptr<Scene> aScene, UUID& outSelected, bool* outClear);
}
