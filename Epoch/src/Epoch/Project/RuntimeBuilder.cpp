#include "epch.h"
#include "RuntimeBuilder.h"
#include <ShlObj.h>
#include "Project.h"
#include "ProjectSerializer.h"
#include "Epoch/Utils/FileSystem.h"
#include "Epoch/Assets/AssetPack/AssetPack.h"

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

		const auto projDir = Project::GetProjectDirectory();
		const auto projFilePath = Project::GetProjectPath();

		std::filesystem::path epochDir = std::filesystem::current_path();
		if (epochDir.stem().string() == "Aeon")
		{
			epochDir = epochDir.parent_path();
		}
		
		FileSystem::CopyContent(epochDir / "Aeon/Resources/Runtime", buildLocation);

		ProjectSerializer projectSerializer(Project::GetActive());
		projectSerializer.SerializeRuntime(buildLocation / "Project.eproj");

		//Build assetpack & shaderpack & copy to build location
		{
			AssetPack::CreateFromActiveProject(aProgress);
			
			FileSystem::CreateDirectory(buildLocation / "Resources/Shaders");
			FileSystem::CopyContent(epochDir / "Aeon/Resources/Shaders", buildLocation / "Resources/Shaders");
		}

		return true;
	}
}