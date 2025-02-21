#pragma once
#include <Epoch/Editor/EditorPanel.h>

namespace Epoch
{
	class ShaderLibraryPanel : public EditorPanel
	{
	public:
		ShaderLibraryPanel(const std::string& aName);
		~ShaderLibraryPanel() = default;

		void OnImGuiRender(bool& aIsOpen) override;

	private:

	};
}
