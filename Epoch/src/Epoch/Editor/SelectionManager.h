#pragma once
#include <Epoch/Core/UUID.h>
#include <unordered_map>

namespace Epoch
{
	enum class SelectionContext
	{
		Scene, ContentBrowser
	};

	class SelectionManager
	{
	public:
		static void Select(SelectionContext aContext, UUID aSelectionID);
		static void Deselect(SelectionContext aContext, UUID aSelectionID);

		static void DeselectAll();
		static void DeselectAll(SelectionContext aContext);

		static bool IsSelected(SelectionContext aContext, UUID aSelectionID);

		static size_t GetSelectionCount(SelectionContext aContext) { return staticContexts[aContext].size(); }
		static const std::vector<UUID>& GetSelections(SelectionContext aContext) { return staticContexts[aContext]; }

	private:
		inline static std::unordered_map<SelectionContext, std::vector<UUID>> staticContexts;
	};
}
