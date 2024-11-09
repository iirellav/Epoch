#pragma once
#include <Epoch/Editor/EditorPanel.h>

namespace Epoch
{
	class ShaderLibraryPanel : public EditorPanel
	{
	public:
		ShaderLibraryPanel() = default;
		~ShaderLibraryPanel() = default;

		void OnImGuiRender(bool& aIsOpen) override;

	private:

	};
}
