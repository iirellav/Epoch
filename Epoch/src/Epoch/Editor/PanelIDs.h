#pragma once
#include <format>
#include <Epoch/Editor/FontAwesome.h>

namespace Epoch
{
#define SCENE_PANEL_ID					std::format("{}  Scene", EP_ICON_TELEVISION).c_str() //EP_ICON_TELEVISION
#define GAME_PANEL_ID					std::format("{}  Game", EP_ICON_GAMEPAD).c_str()
#define SCENE_HIERARCHY_PANEL_ID		std::format("{}  Scene Hierarchy", EP_ICON_LIST).c_str()
#define CONTENT_BROWSER_PANEL_ID		std::format("{}  Content Browser", EP_ICON_DROPBOX).c_str() //EP_ICON_INBOX
#define INSPECTOR_PANEL_ID				std::format("{}  Inspector", EP_ICON_INFO_CIRCLE).c_str()
#define STATISTICS_PANEL_ID				std::format("{}  Statistics", EP_ICON_SIGNAL).c_str()
#define PREFERENCES_PANEL_ID			std::format("{}  Preferences", EP_ICON_COG).c_str()
#define PROJECT_SETTINGS_PANEL_ID		std::format("{}  Project Settings", EP_ICON_COG).c_str()
#define CONSOLE_PANEL_ID				std::format("{}  Console", EP_ICON_LIST_ALT).c_str()
#define ASSET_MANAGER_PANEL_ID			std::format("{}  Asset Registry", EP_ICON_BOOK).c_str()
#define SCRIPT_ENGINE_DEBUG_PANEL_ID	std::format("{}  Script Engine", EP_ICON_LIST_ALT).c_str()
#define SHADER_LIBRARY_PANEL_ID			std::format("{}  Shader Library", EP_ICON_BOOK).c_str()
}
