#include "epch.h"
#include "RuntimeBuilder.h"
#include <ShlObj.h>
#include "Project.h"
#include "ProjectSerializer.h"
#include "Epoch/Utils/FileSystem.h"
#include "Epoch/Assets/AssetPack/AssetPack.h"
#include "Epoch/Rendering/ShaderPack.h"
#include "Epoch/Rendering/Renderer.h"

namespace Epoch
{
	bool RuntimeBuilder::Build(std::atomic<float>& aProgress)
	{
		const auto buildLocation = FileSystem::OpenFolderDialog();

		if (buildLocation.empty())
		{
			return false;
		}

		EPOCH_PROFILE_FUNC();

		CONSOLE_LOG_INFO("Building...");

		const auto projDir = Project::GetProjectDirectory();
		const auto projFilePath = Project::GetProjectPath();

		std::filesystem::path epochDir = std::filesystem::current_path();
		
		//Copy the runtime exe to build location & modify the resources (name, icon) & serialize the project settings
		{
			FileSystem::CopyContent(epochDir / "Resources/Runtime", buildLocation);

			ProjectSerializer projectSerializer(Project::GetActive());
			projectSerializer.SerializeRuntime(buildLocation / "Project.eproj");
		}

		//Build assetpack & shaderpack & copy to build location
		{
			FileSystem::CreateDirectory(buildLocation / "Assets");

			CONSOLE_LOG_INFO("Building asset pack");
			AssetPack::CreateFromActiveProject(aProgress, buildLocation / "Assets");
			CONSOLE_LOG_INFO("Building shader pack");
			ShaderPack::CreateFromLibrary(Renderer::GetShaderLibrary(), buildLocation / "Assets/ShaderPack.esp");
		}

		return true;
	}
}