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
		void DrawRendererPage();
		void DrawPhysicsPage();
	};
}
