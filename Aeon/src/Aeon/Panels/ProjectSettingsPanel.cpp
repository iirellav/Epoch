#include "ProjectSettingsPanel.h"
#include <Epoch/ImGui/ImGui.h>
#include <Epoch/Project/Project.h>
#include <Epoch/Project/ProjectSerializer.h>
#include <Epoch/Physics/PhysicsSystem.h>
#include <Epoch/Physics/PhysicsLayer.h>
#include <Epoch/Editor/PanelIDs.h>

namespace Epoch
{
	ProjectSettingsPanel::ProjectSettingsPanel() : PagePanel(PROJECT_SETTINGS_PANEL_ID)
	{
		myPages.push_back({ "General", [this](){ DrawGeneralPage(); }});
		myPages.push_back({ "Renderer", [this](){ DrawRendererPage(); }});
		myPages.push_back({ "Physics", [this](){ DrawPhysicsPage(); }});
	}

	void ProjectSettingsPanel::DrawGeneralPage()
	{
		ProjectConfig& config = Project::GetActive()->GetConfig();
		
		bool modified = false;

		UI::BeginPropertyGrid();

		modified |= UI::Property_InputText("Product Name", config.productName);
		modified |= UI::Property_InputText("Company Name", config.companyName);
		modified |= UI::Property_InputText("Version", config.version);

		modified |= UI::Property_AssetReference<Scene>("Runtime Start Scene", config.startScene);

		UI::Spacing();

		modified |= UI::Property_InputText("Default Namespace", config.defaultScriptNamespace);

		UI::EndPropertyGrid();

		if (modified)
		{
			ProjectSerializer serializer(Project::GetActive());
			serializer.Serialize(Project::GetProjectPath());
		}
	}

	void ProjectSettingsPanel::DrawRendererPage()
	{
		bool modified = false;



		if (modified)
		{
			ProjectSerializer serializer(Project::GetActive());
			serializer.Serialize(Project::GetProjectPath());
		}
	}

	void ProjectSettingsPanel::DrawPhysicsPage()
	{
		bool modified = false;
		PhysicsSettings& settings = PhysicsSystem::GetSettings();

		UI::BeginPropertyGrid();

		modified |= UI::Property_DragFloat("Fixed Timestep", settings.fixedTimestep, 0.01f, 0.000001f, FLT_MAX, "%.7f");

		CU::Vector3f gravity = settings.gravity / 100.0f;
		if (UI::Property_DragFloat3("Gravity", gravity))
		{
			settings.gravity = gravity * 100.0f;
			modified = true;
		}

		UI::EndPropertyGrid();

		UI::Spacing();

		if (ImGui::TreeNodeEx("Layers", ImGuiTreeNodeFlags_SpanAvailWidth))
		{
			UI::BeginPropertyGrid();

			const auto& layers = PhysicsLayerManager::GetLayers();
			for (size_t i = 0; i < layers.size(); i++)
			{
				if (layers[i].reserved)
				{
					UI::Property_Text(("Builtin Layer " + std::to_string(i)).c_str(), layers[i].name);
				}
				else
				{
					std::string layerName = layers[i].name;
					if (UI::Property_InputText(("User Layer " + std::to_string(i)).c_str(), layerName))
					{
						PhysicsLayerManager::UpdateLayerName((uint32_t)i, layerName);
					}
				}
			}

			UI::EndPropertyGrid();
			ImGui::TreePop();
		}

		if (modified)
		{
			ProjectSerializer serializer(Project::GetActive());
			serializer.Serialize(Project::GetProjectPath());
		}
	}
}
