#include "epch.h"
#include "EditorConsolePanel.h"
#include "Epoch/ImGui/Colors.h"
#include "Epoch/Editor/EditorSettings.h"
#include "Epoch/Editor/PanelIDs.h"

namespace Epoch
{
	static EditorConsolePanel* staticInstance = nullptr;

	EditorConsolePanel::EditorConsolePanel()
	{
		EPOCH_ASSERT(staticInstance == nullptr, "Cant have more than one instances of the editor console panel!");
		staticInstance = this;

		myMessageBuffer.reserve(500);
	}

	EditorConsolePanel::~EditorConsolePanel()
	{
		staticInstance = nullptr;
	}

	void EditorConsolePanel::OnImGuiRender(bool& aIsOpen)
	{
		if (!aIsOpen) return;

		ImGui::Begin(CONSOLE_PANEL_ID);

		ImVec2 consoleSize = ImGui::GetContentRegionAvail();
		consoleSize.y -= 32.0f;

		RenderMenu(ImVec2(consoleSize.x, 28.0f));
		RenderConsole(consoleSize);

		ImGui::End();
	}

	void EditorConsolePanel::OnProjectChanged(const std::shared_ptr<Project>& aProject)
	{
		std::scoped_lock<std::mutex> lock(myMessageBufferMutex);
		myMessageBuffer.clear();
	}

	void EditorConsolePanel::OnScenePlay()
	{
		if (EditorSettings::Get().clearConsoleOnPlay)
		{
			std::scoped_lock<std::mutex> lock(myMessageBufferMutex);
			myMessageBuffer.clear();
			myInfoCount = 0;
			myWarnCount = 0;
			myErrorCount = 0;
		}
	}
	
	static const char* GetMessageType(const ConsoleMessage& aMessage)
	{
		if (aMessage.flags & (int16_t)ConsoleMessageFlags::Debug)	return "Debug";
		if (aMessage.flags & (int16_t)ConsoleMessageFlags::Info)	return "Info";
		if (aMessage.flags & (int16_t)ConsoleMessageFlags::Warning)	return "Warning";
		if (aMessage.flags & (int16_t)ConsoleMessageFlags::Error)	return "Error";
		return "Unknown";
	}

	static uint32_t GetMessageColor(const ConsoleMessage& aMessage)
	{
		if (aMessage.flags & (int16_t)ConsoleMessageFlags::Debug)	return Colors::Theme::debug;
		if (aMessage.flags & (int16_t)ConsoleMessageFlags::Info)	return Colors::Theme::info;
		if (aMessage.flags & (int16_t)ConsoleMessageFlags::Warning)	return Colors::Theme::warning;
		if (aMessage.flags & (int16_t)ConsoleMessageFlags::Error)	return Colors::Theme::error;
		return Colors::Theme::invalid;
	}

	static ImVec4 GetToolbarButtonColor(const bool aValue)
	{
		const auto& style = ImGui::GetStyle();
		return aValue ? style.Colors[ImGuiCol_Header] : style.Colors[ImGuiCol_FrameBg];
	}

	void EditorConsolePanel::RenderMenu(const ImVec2& aSize)
	{
		UI::ScopedStyleStack frame(ImGuiStyleVar_FrameBorderSize, 0.0f, ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::BeginChild("Toolbar", aSize);

		const auto& style = ImGui::GetStyle();
		{
			UI::ScopedStyle noSpace(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 0.0f));

			if (ImGui::Button("Clear", { 75.0f, 24.0f }))
			{
				std::scoped_lock<std::mutex> lock(myMessageBufferMutex);
				myMessageBuffer.clear();
				myInfoCount = 0;
				myWarnCount = 0;
				myErrorCount = 0;
			}

			ImGui::SameLine();

			bool& clearConsoleOnPlay = EditorSettings::Get().clearConsoleOnPlay;
			//const std::string clearOnPlayText = clearConsoleOnPlay ? EP_ICON_CHECK + std::string(" Clear on Play") : "Clear on Play";
			ImVec4 textColor = clearConsoleOnPlay ? style.Colors[ImGuiCol_Text] : style.Colors[ImGuiCol_TextDisabled];
			{
				//UI::ScopedStyle leftAligned(ImGuiStyleVar_ButtonTextAlign, ImVec2(1.0f, 0.5f));
				if (UI::ColoredButton("Clear on Play", GetToolbarButtonColor(clearConsoleOnPlay), textColor, ImVec2(100.0f, 24.0f)))
				{
					clearConsoleOnPlay = !clearConsoleOnPlay;
					EditorSettingsSerializer::SaveSettings();
				}
			}

			{
				bool& collapses = EditorSettings::Get().collapseConsoleMessages;
				ImGui::SameLine();
				if (UI::ColoredButton("Collapse", GetToolbarButtonColor(collapses), collapses ? style.Colors[ImGuiCol_Text] : style.Colors[ImGuiCol_TextDisabled], { 75.0f, 24.0f }))
				{
					collapses = !collapses;
					EditorSettingsSerializer::SaveSettings();
				}
			}
		}

		//{
		//	ImGui::SameLine(ImGui::GetContentRegionAvail().x - 440.0f, 0.0f);
		//	ImGui::SetNextItemWidth(250);
		//	ImGui::InputTextWithHint("##Search", "Search...", &mySearchString, ImGuiInputTextFlags_AutoSelectAll);
		//}

		{
			const ImVec2 buttonSize(55.0f, 24.0f);
			UI::ScopedStyle noSpace(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 0.0f));

			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 170.0f, 0.0f);
			ImVec4 textColor = (myMessageFilters & (int16_t)ConsoleMessageFlags::Info) ? ImColor(Colors::Theme::info) : ImColor(style.Colors[ImGuiCol_TextDisabled]);
			std::string inofCountText = myInfoCount > 99 ? "99+" : std::to_string(myInfoCount);
			std::string infoText = std::format("{} {}", EP_ICON_INFO_CIRCLE, inofCountText);
			if (UI::ColoredButton(infoText.c_str(), GetToolbarButtonColor(myMessageFilters & (int16_t)ConsoleMessageFlags::Info), textColor, buttonSize))
			{
				myMessageFilters ^= (int16_t)ConsoleMessageFlags::Info;
			}

			ImGui::SameLine();
			textColor = (myMessageFilters & (int16_t)ConsoleMessageFlags::Warning) ? ImColor(Colors::Theme::warning) : ImColor(style.Colors[ImGuiCol_TextDisabled]);
			std::string warnCountText = myWarnCount > 99 ? "99+" : std::to_string(myWarnCount);
			std::string warnText = std::format("{} {}", EP_ICON_EXCLAMATION_TRIANGLE, warnCountText);
			if (UI::ColoredButton(warnText.c_str(), GetToolbarButtonColor(myMessageFilters & (int16_t)ConsoleMessageFlags::Warning), textColor, buttonSize))
			{
				myMessageFilters ^= (int16_t)ConsoleMessageFlags::Warning;
			}

			ImGui::SameLine();
			textColor = (myMessageFilters & (int16_t)ConsoleMessageFlags::Error) ? ImColor(Colors::Theme::error) : ImColor(style.Colors[ImGuiCol_TextDisabled]);
			std::string errorCountText = myErrorCount > 99 ? "99+" : std::to_string(myErrorCount);
			std::string errorText = std::format("{} {}", EP_ICON_EXCLAMATION_CIRCLE, errorCountText);
			if (UI::ColoredButton(errorText.c_str(), GetToolbarButtonColor(myMessageFilters & (int16_t)ConsoleMessageFlags::Error), textColor, buttonSize))
			{
				myMessageFilters ^= (int16_t)ConsoleMessageFlags::Error;
			}
		}

		ImGui::EndChild();
	}

	void EditorConsolePanel::RenderConsole(const ImVec2 & aSize)
	{
		static const char* s_Columns[] = { "Type", "Timestamp", "Message" };

		std::vector<ConsoleMessage> messages;
		std::unordered_map<uint64_t, uint32_t> msgMap;
		std::unordered_map<uint64_t, uint32_t> msgCountMap;

		{
			std::scoped_lock<std::mutex> lock(myMessageBufferMutex);
			if (EditorSettings::Get().collapseConsoleMessages)
			{
				std::set<uint64_t> set;

				for (uint32_t i = 0; i < myMessageBuffer.size(); i++)
				{
					const auto& msg = myMessageBuffer[i];

					if (set.find(msg.hash) == set.end())
					{
						msgMap[msg.hash] = (uint32_t)messages.size();
						messages.push_back(msg);
						set.insert(msg.hash);
						msgCountMap[msg.hash] = 1;
					}
					else
					{
						msgCountMap[msg.hash]++;
						messages[msgMap[msg.hash]].time = msg.time;
					}
				}
			}
			else
			{
				messages = myMessageBuffer;
			}
		}

		UI::Table("Console", s_Columns, 3, aSize, [&]()
		{
			float scrollY = ImGui::GetScrollY();
			if (scrollY < myPreviousScrollY)
			{
				myEnableScrollToLatest = false;
			}

			if (scrollY >= ImGui::GetScrollMaxY())
			{
				myEnableScrollToLatest = true;
			}

			myPreviousScrollY = scrollY;

			float rowHeight = 24.0f;
			for (uint32_t i = 0; i < messages.size(); i++)
			{
				const auto& msg = messages[i];

				if (!(myMessageFilters & (int16_t)msg.flags))
				{
					continue;
				}

				ImGui::PushID(&msg);
				
				const bool clicked = UI::TableRowClickable(msg.message.c_str(), rowHeight);

				UI::ShiftCursorY(5.0f);
				UI::Separator(ImVec2(4.0f, ImGui::CalcTextSize(msg.message.c_str()).y), ImColor(GetMessageColor(msg)));
				ImGui::SameLine();
				UI::ShiftCursorY(-5.0f);
				ImGui::Text(GetMessageType(msg));
				ImGui::TableNextColumn();
				UI::ShiftCursorX(4.0f);

				std::stringstream timeString;
				tm* timeBuffer = localtime(&msg.time);
				timeString << std::put_time(timeBuffer, "%T");
				ImGui::Text(timeString.str().c_str());

				ImGui::TableNextColumn();
				UI::ShiftCursorX(4.0f);
				uint32_t msgCount = msgCountMap[msg.hash];
				if (msgCount > 1u)
				{
					ImGui::TextWrapped("[%u] %s", msgCount, msg.message.c_str());
				}
				else
				{
					ImGui::TextWrapped(msg.message.c_str());
				}

				if (i == messages.size() - 1 && myScrollToLatest)
				{
					ImGui::ScrollToItem();
					myScrollToLatest = false;
				}

				ImGui::PopID();
			}
		});
	}

	void EditorConsolePanel::PushMessage(const ConsoleMessage& aMessage)
	{
		if (staticInstance == nullptr) return;

		{
			std::scoped_lock<std::mutex> lock(staticInstance->myMessageBufferMutex);
			staticInstance->myMessageBuffer.push_back(aMessage);

			switch ((ConsoleMessageFlags)aMessage.flags)
			{
			case ConsoleMessageFlags::Info:		++staticInstance->myInfoCount; break;
			case ConsoleMessageFlags::Warning:	++staticInstance->myWarnCount; break;
			case ConsoleMessageFlags::Error:	++staticInstance->myErrorCount; break;
			}
		}

		if (staticInstance->myEnableScrollToLatest)
		{
			staticInstance->myScrollToLatest = true;
		}
	}
}
