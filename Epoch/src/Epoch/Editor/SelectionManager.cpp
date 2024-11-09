#include "epch.h"
#include "SelectionManager.h"

namespace Epoch
{
	void Epoch::SelectionManager::Select(SelectionContext aContext, UUID aSelectionID)
	{
		auto& selection = staticContexts[aContext];

		auto it = std::find(selection.begin(), selection.end(), aSelectionID);
		if (it != selection.end()) return;

		selection.push_back(aSelectionID);
	}

	void Epoch::SelectionManager::Deselect(SelectionContext aContext, UUID aSelectionID)
	{
		auto& selection = staticContexts[aContext];

		auto it = std::find(selection.begin(), selection.end(), aSelectionID);
		if (it == selection.end()) return;

		selection.erase(it);
	}

	void Epoch::SelectionManager::DeselectAll()
	{
		for (auto& [contexts, selections] : staticContexts)
		{
			selections.clear();
		}
	}

	void Epoch::SelectionManager::DeselectAll(SelectionContext aContext)
	{
		auto& selection = staticContexts[aContext];
		selection.clear();
	}

	bool Epoch::SelectionManager::IsSelected(SelectionContext aContext, UUID aSelectionID)
	{
		const auto& selection = staticContexts[aContext];
		return std::find(selection.begin(), selection.end(), aSelectionID) != selection.end();
	}
}