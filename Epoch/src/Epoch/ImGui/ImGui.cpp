#include "epch.h"
#include "ImGui.h"

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
}
