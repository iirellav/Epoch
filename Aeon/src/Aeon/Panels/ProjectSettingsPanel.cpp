#include "ProjectSettingsPanel.h"
#include <Epoch/ImGui/ImGui.h>
#include <Epoch/Project/Project.h>
#include <Epoch/Project/ProjectSerializer.h>
#include <Epoch/Physics/PhysicsSystem.h>
#include <Epoch/Editor/FontAwesome.h>

namespace Epoch
{
	ProjectSettingsPanel::ProjectSettingsPanel() : PagePanel(std::format("{}  Project Settings", EP_ICON_COG).c_str())
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

		modified |= UI::Property_DragFloat("Fixed Timestep", settings.fixedTimestep, 0.01f, 0.000001, FLT_MAX, "%.7f");

		CU::Vector3f gravity = settings.gravity / 100.0f;
		if (UI::Property_DragFloat3("Gravity", gravity))
		{
			settings.gravity = gravity * 100.0f;
			modified = true;
		}

		UI::EndPropertyGrid();

		if (modified)
		{
			ProjectSerializer serializer(Project::GetActive());
			serializer.Serialize(Project::GetProjectPath());
		}
	}
}
