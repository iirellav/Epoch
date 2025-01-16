#include "PreferencesPanel.h"
#include <CommonUtilities/Math/CommonMath.hpp>
#include <Epoch/ImGui/ImGui.h>
#include <Epoch/Editor/EditorSettings.h>
#include <Epoch/Editor/PanelIDs.h>

namespace Epoch
{
	PreferencesPanel::PreferencesPanel() : PagePanel(PREFERENCES_PANEL_ID)
	{
		myPages.push_back({ "General", [this](){ DrawGeneralPage(); }});
		myPages.push_back({ "Level Editor", [this](){ DrawLevelEditorPage(); }});
		myPages.push_back({ "Renderer", [this](){ DrawRendererPage(); }});
		myPages.push_back({ "Scripting", [this](){ DrawScriptingPage(); }});
	}

	void PreferencesPanel::DrawGeneralPage()
	{
		bool modified = false;
		auto& settings = EditorSettings::Get();
		
		UI::BeginPropertyGrid();

		modified |= UI::Property_Checkbox("Load Previous Project on Startup", settings.loadLastOpenProject);

		UI::EndPropertyGrid();

		if (UI::PropertyGridHeader("Autosaving"))
		{
			UI::BeginPropertyGrid();
			
			modified |= UI::Property_Checkbox("Autosave Enabled", settings.autosaveEnabled);

			int autosaveIntervalMinutes = (int)((float)settings.autosaveIntervalSeconds * 0.016667f);
			if (UI::Property_InputInt("Autosave Interval (minutes)", autosaveIntervalMinutes, 5, 10, 5, 60))
			{
				modified = true;
				settings.autosaveIntervalSeconds = (int)((float)autosaveIntervalMinutes * 60.0f);
			}

			modified |= UI::Property_Checkbox("Autosave Scene on Play", settings.autoSaveSceneBeforePlay);

			UI::EndPropertyGrid();

			ImGui::TreePop();
		}

		if (UI::PropertyGridHeader("Console"))
		{
			UI::BeginPropertyGrid();

			modified |= UI::Property_Checkbox("Clear Console on Play", settings.clearConsoleOnPlay);
			modified |= UI::Property_Checkbox("Collapse Console Messages", settings.collapseConsoleMessages);
			
			UI::EndPropertyGrid();

			ImGui::TreePop();
		}

		if (modified)
		{
			EditorSettingsSerializer::SaveSettings();
		}
	}

	void PreferencesPanel::DrawLevelEditorPage()
	{
		bool modified = false;
		auto& settings = EditorSettings::Get();
		
		UI::BeginPropertyGrid();

		modified |= UI::Property_Checkbox("Create Entities at Origin", settings.createEntitiesAtOrigin);

		const char* multiTransformTargetStrings[] = { "Median Point", "Individual Origins" };
		uint32_t currentTarget = (int)settings.multiTransformTarget;
		if (UI::Property_Dropdown("Multi Transform Target", multiTransformTargetStrings, 2, currentTarget))
		{
			modified = true;
			settings.multiTransformTarget = (TransformationTarget)currentTarget;
		}

		UI::EndPropertyGrid();

		UI::Spacing();

		if (UI::PropertyGridHeader("Snap Values"))
		{
			UI::BeginPropertyGrid();

			float translationSnapValueMeter = settings.translationSnapValue * 0.01f;
			if (UI::Property_DragFloat("Translate", translationSnapValueMeter))
			{
				modified = true;
				settings.translationSnapValue = translationSnapValueMeter * 100.0f;
			}

			modified |= UI::Property_DragFloat("Rotate", settings.rotationSnapValue);
			modified |= UI::Property_DragFloat("Scale", settings.scaleSnapValue);

			UI::EndPropertyGrid();

			ImGui::TreePop();
		}

		UI::Spacing();

		if (UI::PropertyGridHeader("Grid"))
		{
			UI::BeginPropertyGrid();

			modified |= UI::Property_Checkbox("Show Grid", settings.gridEnabled);
			modified |= UI::Property_SliderFloat("Opacity", settings.gridOpacity, 0.0f, 1.0f, NULL, ImGuiSliderFlags_NoInput | ImGuiSliderFlags_AlwaysClamp);

			CU::Vector3f gridOffset = settings.gridOffset * 0.01f;
			modified |= UI::Property_DragFloat3("Offset", gridOffset, 0.2f);
			settings.gridOffset = gridOffset * 100.0f;

			CU::Vector2i gridSize = CU::Vector2i((int)settings.gridSize.x, (int)settings.gridSize.y);
			modified |= UI::Property_DragInt2("Size", gridSize, 1, 0, 64);
			settings.gridSize = CU::Vector2f((float)gridSize.x, (float)gridSize.y);

			bool gridPlaneX = settings.gridPlane == GridPlane::X;
			bool gridPlaneY = settings.gridPlane == GridPlane::Y;
			bool gridPlaneZ = settings.gridPlane == GridPlane::Z;

			UI::BeginCheckboxGroup("Grid Plane");

			if (UI::Property_GroupCheckbox("X", gridPlaneX))
			{
				modified = true;
				settings.gridPlane = GridPlane::X;
			}

			if (UI::Property_GroupCheckbox("Y", gridPlaneY))
			{
				modified = true;
				settings.gridPlane = GridPlane::Y;
			}

			if (UI::Property_GroupCheckbox("Z", gridPlaneZ))
			{
				modified = true;
				settings.gridPlane = GridPlane::Z;
			}

			UI::EndCheckboxGroup();

			UI::EndPropertyGrid();

			ImGui::TreePop();
		}

		if (modified)
		{
			EditorSettingsSerializer::SaveSettings();
		}
	}

	void PreferencesPanel::DrawRendererPage()
	{
		bool modified = false;
		auto& settings = EditorSettings::Get();

		UI::BeginPropertyGrid();

		modified |= UI::Property_Checkbox("Automatically Reload Shaders", settings.automaticallyReloadShaders);

		UI::EndPropertyGrid();

		if (modified)
		{
			EditorSettingsSerializer::SaveSettings();
		}
	}

	void PreferencesPanel::DrawScriptingPage()
	{
		bool modified = false;
		auto& settings = EditorSettings::Get();

		UI::BeginPropertyGrid();

		modified |= UI::Property_Checkbox("Automatically Reload Script Assembly", settings.automaticallyReloadScriptAssembly, "The script assembly gets automatically reloaded when built.");

		if (settings.automaticallyReloadScriptAssembly)
		{
			const char* optionsStrings[] = { "Stop Playing", "Reload After Finished Playing" };
			uint32_t currentOptions = (int)settings.reloadScriptAssemblyWhilePlaying;
			if (UI::Property_Dropdown("Reload Script Assembly While Playing", optionsStrings, 2, currentOptions, "How should reloading the script assembly be handled when game is running"))
			{
				modified = true;
				settings.reloadScriptAssemblyWhilePlaying = (ReloadScriptAssemblyWhilePlaying)currentOptions;
			}
		}

		UI::EndPropertyGrid();

		if (modified)
		{
			EditorSettingsSerializer::SaveSettings();
		}
	}
}
