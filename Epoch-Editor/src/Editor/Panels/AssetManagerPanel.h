#pragma once
#include <Epoch/Editor/EditorPanel.h>

namespace Epoch
{
	class AssetManagerPanel : public EditorPanel
	{
	public:
		AssetManagerPanel(const std::string& aName);
		~AssetManagerPanel() = default;

		void OnImGuiRender(bool& aIsOpen) override;
	};
}
