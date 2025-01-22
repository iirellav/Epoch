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

namespace Epoch
{
	bool RuntimeBuilder::Build(std::filesystem::path aBuildLocation)
	{
		EPOCH_PROFILE_FUNC();

		Timer buildTimer;

		const auto projDir = Project::GetProjectDirectory();
		const auto projFilePath = Project::GetProjectPath();

		std::filesystem::path epochDir = std::filesystem::current_path();
		
		FileSystem::CopyContent(epochDir / "Resources/Runtime", aBuildLocation);

		//Set icon and other app variables

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