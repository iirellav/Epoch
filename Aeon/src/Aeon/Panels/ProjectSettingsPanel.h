#pragma once
#include "Aeon/Panels/PagePanel.h"

namespace Epoch
{
	class ProjectSettingsPanel : public PagePanel
	{
	public:
		ProjectSettingsPanel();
		~ProjectSettingsPanel() override = default;

	private:
		void DrawGeneralPage();
		void DrawBuildPage();
		void DrawRendererPage();
		void DrawPhysicsPage();
		void DrawScriptingPage();
	};
}
