#pragma once
#include "Editor/Panels/PagePanel.h"

namespace Epoch
{
	class PreferencesPanel : public PagePanel
	{
	public:
		PreferencesPanel(const std::string& aName);
		~PreferencesPanel() override = default;
		
	private:
		void DrawGeneralPage();
		void DrawLevelEditorPage();
		void DrawRendererPage();
		void DrawScriptingPage();
	};
}
