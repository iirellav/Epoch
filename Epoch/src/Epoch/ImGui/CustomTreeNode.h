#pragma once
#include "ImGui.h"

namespace Epoch
{
	class Texture2D;
}

namespace ImGui
{
	bool TreeNodeWithIcon(std::shared_ptr<Epoch::Texture2D> aIcon, ImGuiID aId, ImGuiTreeNodeFlags aFlags, const char* label, const char* aLabelEnd, ImColor aIconTint = IM_COL32_WHITE);
	bool TreeNodeWithIcon(std::shared_ptr<Epoch::Texture2D> aIcon, const char* aLabel, ImGuiTreeNodeFlags aFlags, ImColor aIconTint = IM_COL32_WHITE);
}
