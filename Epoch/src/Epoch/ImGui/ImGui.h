#pragma once
#include "Epoch/ImGui/UICore.h"
#include "Epoch/ImGui/ImGuiUtilities.h"
#include "Epoch/ImGui/ImGuiFonts.h"
#include "Epoch/ImGui/Colors.h"

namespace Epoch::UI
{
	bool IsMatchingSearch(const std::string& aItem, const std::string& aSearchQuery, bool aCaseSensitive = false, bool aStripWhiteSpaces = true);
}
