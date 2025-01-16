#pragma once
#include <string>
#include <memory>
#include <filesystem>
#include <CommonUtilities/Math/Vector/Vector3.hpp>
#include <Epoch/Debug/Log.h>
#include "Epoch/Assets/AssetManager/EditorAssetManager.h"

namespace Epoch
{
	struct ProjectConfig
	{
		std::string productName = "Untitled";
		std::string name = "Untitled";
		std::string companyName = "";
		std::string version = "1.0.0";

		AssetHandle startScene = 0;

		std::filesystem::path autosaveDirectory = "Autosaves";

		std::filesystem::path assetDirectory = "Assets";
		std::filesystem::path assetRegistryPath = "Regs/AssetRegistry.epr";
		
		std::string scriptModulePath = "Scripts/Binaries";
		std::string defaultScriptNamespace;

		// Not serialized
		std::filesystem::path projectFileName;
		std::filesystem::path projectDirectory;
	};

	struct RecentScene
	{
		std::string name = "";
		std::string filePath = "";
		time_t lastOpened = 0;
	};

	struct ProjectUserConfig
	{
		std::filesystem::path startScene;

		CU::Vector3f editorCameraPosition;
		CU::Vector3f editorCameraRotation;

		std::map<time_t, RecentScene, std::greater<time_t>> recentScenes;
	};

	class Project
	{
	public:
		Project() = default;
		~Project() = default;

		ProjectConfig& GetConfig() { return myConfig; }
		ProjectUserConfig& GetUserConfig() { return myUserConfig; }
		static std::shared_ptr<Project> GetActive() { return staticActiveProject; }

		static void SetActive(std::shared_ptr<Project> aProject);
		static void SetActiveRuntime(std::shared_ptr<Project> aProject);

		inline static std::shared_ptr<AssetManagerBase> GetAssetManager() { return staticAssetManager; }
		inline static std::shared_ptr<EditorAssetManager> GetEditorAssetManager() { return std::static_pointer_cast<EditorAssetManager>(staticAssetManager); }

		static const std::string& GetProductName()
		{
			EPOCH_ASSERT(staticActiveProject, "No active project!");
			return staticActiveProject->GetConfig().productName;
		}

		static const std::string GetProjectName()
		{
			EPOCH_ASSERT(staticActiveProject, "No active project!");
			return staticActiveProject->GetConfig().name;
		}

		static std::filesystem::path GetProjectDirectory()
		{
			EPOCH_ASSERT(staticActiveProject, "No active project!");
			return staticActiveProject->GetConfig().projectDirectory;
		}

		static std::filesystem::path GetProjectPath()
		{
			EPOCH_ASSERT(staticActiveProject, "No active project!");
			return staticActiveProject->GetConfig().projectDirectory / staticActiveProject->GetConfig().projectFileName;
		}

		static std::filesystem::path GetAutosaveDirectory()
		{
			EPOCH_ASSERT(staticActiveProject, "No active project!");
			return staticActiveProject->GetConfig().projectDirectory / staticActiveProject->GetConfig().autosaveDirectory;
		}

		static std::filesystem::path GetAssetDirectory()
		{
			EPOCH_ASSERT(staticActiveProject, "No active project!");
			return staticActiveProject->GetConfig().projectDirectory / staticActiveProject->GetConfig().assetDirectory;
		}
		
		static std::filesystem::path GetAssetRegistryPath()
		{
			EPOCH_ASSERT(staticActiveProject, "No active project!");
			return staticActiveProject->GetConfig().projectDirectory / staticActiveProject->GetConfig().assetRegistryPath;
		}

		static std::filesystem::path GetScriptModulePath()
		{
			EPOCH_ASSERT(staticActiveProject, "No active project!");
			return std::filesystem::path(staticActiveProject->GetConfig().projectDirectory) / staticActiveProject->GetConfig().scriptModulePath;
		}

		static std::filesystem::path GetScriptModuleFilePath()
		{
			EPOCH_ASSERT(staticActiveProject, "No active project!");
			return GetScriptModulePath() / fmt::format("{}.dll", GetProjectName());
		}

		static std::filesystem::path GetScriptSolutionPath()
		{
			EPOCH_ASSERT(staticActiveProject, "No active project!");
			return GetProjectDirectory() / std::format("{}.sln", GetProjectName());
		}

	private:
		ProjectConfig myConfig;
		ProjectUserConfig myUserConfig;

		inline static std::shared_ptr<AssetManagerBase> staticAssetManager;
		inline static std::shared_ptr<Project> staticActiveProject;

		friend class ProjectSerializer;
	};
}
