#include "epch.h"
#include "RuntimeBuilder.h"
#include <ShlObj.h>
#include "Project.h"
#include "ProjectSerializer.h"
#include "Epoch/Utils/FileSystem.h"
#include "Epoch/Assets/AssetPack/AssetPack.h"
#include "Epoch/Rendering/ShaderPack.h"
#include "Epoch/Rendering/Renderer.h"
#include "Epoch/Debug/Timer.h"
#include "Epoch/Editor/EditorSettings.h"

namespace Epoch
{
	bool RuntimeBuilder::Build(std::filesystem::path aBuildLocation)
	{
		EPOCH_PROFILE_FUNC();

		Timer buildTimer;

		const ProjectConfig& configs = Project::GetActive()->GetConfig();

		const auto projDir = Project::GetProjectDirectory();
		const auto projFilePath = Project::GetProjectPath();

		std::filesystem::path epochDir = std::filesystem::current_path();
		
		FileSystem::CopyContent(epochDir / "Resources/Runtime", aBuildLocation);

		if (FileSystem::Exists("ExternalTools/rcedit.exe"))
		{
			const std::filesystem::path rcEditPath = std::filesystem::absolute("ExternalTools/rcedit.exe");

			std::ifstream stream(aBuildLocation / "SetResources.bat");
			EPOCH_ASSERT(stream.is_open(), "Could not open project file!");
			std::stringstream ss;
			ss << stream.rdbuf();
			stream.close();

			std::string str = ss.str();
			CU::ReplaceToken(str, "$RCEDIT$", rcEditPath.string());
			CU::ReplaceToken(str, "$ICON_PATH$", configs.iconPath.string());
			CU::ReplaceToken(str, "$PRUDUCT_NAME$", configs.productName);

			std::ofstream ostream(aBuildLocation / "SetResources.bat");
			ostream << str;
			ostream.close();
			
			WinExec((aBuildLocation / "SetResources.bat").string().c_str(), SW_HIDE);
		}

		ProjectSerializer projectSerializer(Project::GetActive());
		projectSerializer.SerializeRuntime(aBuildLocation / "Project.eproj");

		FileSystem::CopyFile("Resources/Scripts/Epoch-ScriptCore.dll", aBuildLocation);

		FileSystem::CreateDirectory(aBuildLocation / "Assets");

		AssetPack::CreateFromActiveProject(aBuildLocation / "Assets");
		ShaderPack::CreateFromLibrary(Renderer::GetShaderLibrary(), aBuildLocation / "Assets/ShaderPack.esp");

		CONSOLE_LOG_INFO("Build took {}s to complete", buildTimer.Elapsed());

		return true;
	}
}