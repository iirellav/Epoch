#pragma once
#include "Epoch/Editor/EditorPanel.h"
#include "ConsoleMessage.h"
#include "Epoch/ImGui/ImGui.h"

namespace Epoch
{
	class EditorConsolePanel : public EditorPanel
	{
	public:
		EditorConsolePanel(const std::string& aName);
		~EditorConsolePanel() override;
		
		void OnImGuiRender(bool& aIsOpen) override;
		
		void OnEvent(Event& aEvent) override;

		void OnProjectChanged(const std::shared_ptr<Project>& aProject) override;

	private:
		void RenderMenu(const ImVec2& aSize);
		void RenderConsole(const ImVec2& aSize);

		static void PushMessage(const ConsoleMessage& aMessage);

	private:
		std::mutex myMessageBufferMutex;
		std::vector<ConsoleMessage> myMessageBuffer;
		unsigned myInfoCount = 0;
		unsigned myWarnCount = 0;
		unsigned myErrorCount = 0;

		std::string mySearchString = "";

		bool myEnableScrollToLatest = true;
		bool myScrollToLatest = false;
		float myPreviousScrollY = 0.0f;

		int16_t myMessageFilters = (int16_t)ConsoleMessageFlags::All;

		friend class EditorConsoleSink;
	};
}
