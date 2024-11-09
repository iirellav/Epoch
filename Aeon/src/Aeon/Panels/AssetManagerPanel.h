#pragma once
#include <Epoch/Editor/EditorPanel.h>

namespace Epoch
{
	class AssetManagerPanel : public EditorPanel
	{
	public:
		AssetManagerPanel() = default;
		~AssetManagerPanel() = default;

		void OnImGuiRender(bool& aIsOpen) override;

	private:

	};
}
