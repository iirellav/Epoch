#include "epch.h"
#include "ImGui.h"
#include "CustomTreeNode.h"

namespace Epoch::UI
{
	bool IsMatchingSearch(const std::string& aItem, const std::string& aSearchQuery, bool aCaseSensitive, bool aStripWhiteSpaces)
	{
		if (aSearchQuery.empty()) return true;
		if (aItem.empty()) return false;

		std::string item = aItem;
		std::string searchString = aSearchQuery;

		if (aStripWhiteSpaces)
		{
			item.erase(std::remove_if(item.begin(), item.end(), ::isspace), item.end());
			searchString.erase(std::remove_if(searchString.begin(), searchString.end(), ::isspace), searchString.end());
		}

		if (!aCaseSensitive)
		{
			item = CU::ToLower(item);
			searchString = CU::ToLower(searchString);
		}

		return item.find(searchString) != std::string::npos;
	}

	bool TreeNode(const std::string& aId, const std::string& aLabel, ImGuiTreeNodeFlags aFlags, const std::shared_ptr<Texture2D>& aIcon)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
		{
			return false;
		}

		return ImGui::TreeNodeWithIcon(aIcon, window->GetID(aId.c_str()), aFlags, aLabel.c_str(), NULL);
	}
}
