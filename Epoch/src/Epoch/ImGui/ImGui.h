#pragma once
#include "UICore.h"
#include "ImGuiUtilities.h"
#include "ImGuiFonts.h"
#include "Colors.h"

namespace Epoch::UI
{
	bool IsMatchingSearch(const std::string& aItem, const std::string& aSearchQuery, bool aCaseSensitive = false, bool aStripWhiteSpaces = true);

	bool TreeNode(const std::string& aId, const std::string& aLabel, ImGuiTreeNodeFlags aFlags = 0, const std::shared_ptr<Texture2D>& aIcon = nullptr);
}
