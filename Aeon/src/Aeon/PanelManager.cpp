#include "PanelManager.h"
#include <filesystem>
#include <Epoch/Debug/Profiler.h>
#include <Epoch/Utils/FileSystem.h>
#include <Epoch/Utils/YAMLSerializationHelpers.h>

namespace Epoch
{
	void PanelManager::OnImGuiRender()
	{
		for (auto& panelMap : myPanels)
		{
			for (auto& [id, panelData] : panelMap)
			{
				bool closedThisFrame = false;

				if (panelData.isOpen)
				{
					panelData.panel->OnImGuiRender(panelData.isOpen);
					closedThisFrame = !panelData.isOpen;
				}

				if (closedThisFrame)
				{
					Serialize();
				}
			}
		}
	}

	void PanelManager::OnEvent(Event& aEvent)
	{
		for (auto& panelMap : myPanels)
		{
			for (auto& [id, panelData] : panelMap)
			{
				panelData.panel->OnEvent(aEvent);
			}
		}
	}

	void PanelManager::OnProjectChanged(const std::shared_ptr<Project>& aProject)
	{
		for (auto& panelMap : myPanels)
		{
			for (auto& [id, panelData] : panelMap)
			{
				panelData.panel->OnProjectChanged(aProject);
			}
		}
	}

	void PanelManager::OnSceneChanged(const std::shared_ptr<Scene>& aScene)
	{
		if (aScene)
		{
			EPOCH_ASSERT(myOnEntityDestroyedCallback, "Panel manager didn't have a callback for destroying entities when scene changed!");
			aScene->SetEntityDestroyedCallback(myOnEntityDestroyedCallback);
		}

		for (auto& panelMap : myPanels)
		{
			for (auto& [id, panelData] : panelMap)
			{
				panelData.panel->OnSceneChanged(aScene);
			}
		}
	}

	static std::filesystem::path staticEditorLayoutPath = "";

	void PanelManager::Serialize()
	{
		EPOCH_PROFILE_FUNC();

		if (staticEditorLayoutPath.empty())
		{
			staticEditorLayoutPath = std::filesystem::absolute("Config");
			if (!std::filesystem::exists(staticEditorLayoutPath)) std::filesystem::create_directory(staticEditorLayoutPath);
			staticEditorLayoutPath /= "EditorLayout.yaml";
		}

		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Panels" << YAML::Value << YAML::BeginSeq;
		{
			for (size_t category = 0; category < myPanels.size(); category++)
			{
				for (const auto& [panelName, panel] : myPanels[category])
				{
					out << YAML::BeginMap;
					out << YAML::Key << "Name" << YAML::Value << panelName;
					out << YAML::Key << "IsOpen" << YAML::Value << panel.isOpen;
					out << YAML::EndMap;
				}
			}
		}
		out << YAML::EndSeq;

		out << YAML::EndMap;

		std::ofstream fout(staticEditorLayoutPath);
		fout << out.c_str();
		fout.close();
	}

	void PanelManager::Deserialize()
	{
		EPOCH_PROFILE_FUNC();

		if (staticEditorLayoutPath.empty())
		{
			staticEditorLayoutPath = std::filesystem::absolute("Config");
			if (!std::filesystem::exists(staticEditorLayoutPath)) std::filesystem::create_directory(staticEditorLayoutPath);
			staticEditorLayoutPath /= "EditorLayout.yaml";
		}

		if (!FileSystem::Exists(staticEditorLayoutPath)) return;

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(staticEditorLayoutPath.string());
		}
		catch (YAML::ParserException e)
		{
			LOG_ERROR("Failed to load .yaml file '{}'\n     {}", staticEditorLayoutPath.string(), e.what());
			return;
		}

		if (!data["Panels"])
		{
			LOG_ERROR("Failed to load EditorLayout.yaml from {}!", staticEditorLayoutPath.parent_path().string());
			return;
		}

		for (auto panelNode : data["Panels"])
		{
			PanelData* panelData = GetPanelData(panelNode["Name"].as<std::string>(""));

			if (panelData == nullptr)
			{
				continue;
			}

			panelData->isOpen = panelNode["IsOpen"].as<bool>(panelData->isOpen);
		}
	}

	PanelData* PanelManager::GetPanelData(const std::string& aName)
	{
		for (auto& panelMap : myPanels)
		{
			if (panelMap.find(aName) == panelMap.end())
			{
				continue;
			}

			return &panelMap.at(aName);
		}

		return nullptr;
	}
}
