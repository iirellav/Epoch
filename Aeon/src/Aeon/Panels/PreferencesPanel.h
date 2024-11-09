#pragma once
#include "Aeon/Panels/PagePanel.h"

namespace Epoch
{
	class PreferencesPanel : public PagePanel
	{
	public:
		PreferencesPanel();
		~PreferencesPanel() override = default;
		
	private:
		void DrawGeneralPage();
		void DrawLevelEditorPage();
		void DrawRendererPage();
		void DrawScriptingPage();
	};
}
