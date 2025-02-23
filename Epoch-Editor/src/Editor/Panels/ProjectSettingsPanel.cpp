#include "ProjectSettingsPanel.h"
#include <Epoch/ImGui/ImGui.h>
#include <Epoch/Project/Project.h>
#include <Epoch/Project/ProjectSerializer.h>
#include <Epoch/Physics/PhysicsSystem.h>
#include <Epoch/Physics/PhysicsLayer.h>
#include <Epoch/Editor/PanelIDs.h>

namespace Epoch
{
	ProjectSettingsPanel::ProjectSettingsPanel(const std::string& aName) : PagePanel(aName)
	{
		myPages.push_back({ "Build", [this](){ DrawBuildPage(); }});
		myPages.push_back({ "Renderer", [this](){ DrawRendererPage(); }});
		myPages.push_back({ "Physics", [this](){ DrawPhysicsPage(); }});
		myPages.push_back({ "Scripting", [this](){ DrawScriptingPage(); }});
	}

	void ProjectSettingsPanel::DrawBuildPage()
	{
		ProjectConfig& config = Project::GetActive()->GetConfig();
		
		bool modified = false;

		UI::BeginPropertyGrid();
		
		modified |= UI::Property_InputText("Product Name", config.productName);
		modified |= UI::Property_AssetReference<AssetType::Texture>("Icon Path", config.appIcon);

		//modified |= UI::Property_InputText("Company Name", config.companyName);
		//modified |= UI::Property_InputText("Version", config.version);

		UI::EndPropertyGrid();

		ImGui::Spacing();
		ImGui::Separator();

		UI::BeginPropertyGrid();
		
		modified |= UI::Property_AssetReference<AssetType::Scene>("Start Scene", config.startScene);

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
		ImGui::Separator();

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
					if (UI::Property_InputText(("User Layer " + std::to_string(i)).c_str(), layerName, ImGuiInputTextFlags_EnterReturnsTrue))
					{
						PhysicsLayerManager::UpdateLayerName((uint32_t)i, layerName);
					}
				}
			}

			UI::EndPropertyGrid();
			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Layer Collisions", ImGuiTreeNodeFlags_SpanAvailWidth))
		{
			const auto& layers = PhysicsLayerManager::GetLayers();

			for (size_t i = 0; i < layers.size(); i++)
			{
				const auto& layer0 = layers[i];

				if (layer0.name == "")
				{
					continue;
				}

				ImGui::PushID(layer0.name.c_str());
				if (ImGui::TreeNodeEx(layer0.name.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
				{
					UI::BeginPropertyGrid();

					for (size_t j = 0; j < layers.size(); j++)
					{
						const auto& layer1 = layers[j];

						if (layer1.name == "")
						{
							continue;
						}

						bool shouldCollide = PhysicsLayerManager::ShouldCollide(layer0.layerID, layer1.layerID);
						if (UI::Property_Checkbox(layer1.name.c_str(), shouldCollide))
						{
							PhysicsLayerManager::SetLayerCollision(layer0.layerID, layer1.layerID, shouldCollide);
						}
					}

					UI::EndPropertyGrid();

					ImGui::TreePop();
				}
				ImGui::PopID();
			}

			ImGui::TreePop();
		}

		if (modified)
		{
			ProjectSerializer serializer(Project::GetActive());
			serializer.Serialize(Project::GetProjectPath());
		}
	}
	
	void ProjectSettingsPanel::DrawScriptingPage()
	{
		ProjectConfig& config = Project::GetActive()->GetConfig();
		
		bool modified = false;

		UI::BeginPropertyGrid();
		
		modified |= UI::Property_InputText("Default Namespace", config.defaultScriptNamespace);

		UI::EndPropertyGrid();

		if (modified)
		{
			ProjectSerializer serializer(Project::GetActive());
			serializer.Serialize(Project::GetProjectPath());
		}
	}
}
