#include "epch.h"
#include "RuntimeBuilder.h"
#include <ShlObj.h>
#include "Epoch/Utils/FileSystem.h"
#include "Project.h"
#include "ProjectSerializer.h"

namespace Epoch
{
	bool RuntimeBuilder::Build()
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

		//Build runtime proj
		{
			EPOCH_PROFILE_SCOPE("RuntimeBuilder::Build::MSBuild()");

			TCHAR programFilesFilePath[MAX_PATH];
			SHGetSpecialFolderPath(0, programFilesFilePath, CSIDL_PROGRAM_FILES, FALSE);
			std::filesystem::path msBuildPath = std::filesystem::path(programFilesFilePath) / "Microsoft Visual Studio" / "2022" / "Community" / "Msbuild" / "Current" / "Bin" / "MSBuild.exe";

			const std::filesystem::path projFileDir = epochDir / "Epoch-Runtime";
			const std::string projFile = (projFileDir / "Epoch-Runtime.vcxproj").string();
			std::string command = fmt::format("cd \"{}\" && \"{}\" \"{}\" -property:Configuration=Dist -property:Platform=x64", projFileDir, msBuildPath.string(), projFile);
			//std::system(command.c_str());
			WinExec(command.c_str(), SW_HIDE);

			if (!FileSystem::Exists(epochDir / "bin/Dist-x86_64/Epoch-Runtime/Runtime.exe"))
			{
				LOG_ERROR("Failed to build!");
				std::system(command.c_str());
			}

			if (!FileSystem::Exists(epochDir / "bin/Dist-x86_64/Epoch-Runtime/Runtime.exe"))
			{
				LOG_ERROR("Failed to build!");
				return false;
			}
		}
		
		FileSystem::CopyContent(epochDir / "Aeon/Resources/Runtime", buildLocation);

		ProjectSerializer projectSerializer(Project::GetActive());
		projectSerializer.SerializeRuntime(buildLocation / "Project.eproj");

		//Copy the .exe & the script .dll:s to build location
		{
			FileSystem::CopyFile(epochDir / "bin/Dist-x86_64/Epoch-Runtime/Runtime.exe", buildLocation);
			FileSystem::RenameFilename(buildLocation / "Runtime.exe", Project::GetProductName());

			FileSystem::CreateDirectory(buildLocation / "Scripts/Binaries");
			FileSystem::CopyFile(projDir / "Scripts/Binaries/Epoch-ScriptCore.dll", buildLocation / "Scripts/Binaries");
			FileSystem::CopyFile(projDir / "Scripts/Binaries/" / (Project::GetProjectName() + ".dll"), buildLocation / "Scripts/Binaries");
		}
		
		//Build assetpack & shaderpack & copy to build location
		{
			//TODO: Build assetpack & shaderpack & copy to build location

			FileSystem::CreateDirectory(buildLocation / "Assets");
			FileSystem::CopyContent(projDir / "Assets", buildLocation / "Assets");

			FileSystem::CreateDirectory(buildLocation / "Regs");
			FileSystem::CopyContent(projDir / "Regs", buildLocation / "Regs");
			
			FileSystem::CreateDirectory(buildLocation / "Resources/Shaders");
			FileSystem::CopyContent(epochDir / "Aeon/Resources/Shaders", buildLocation / "Resources/Shaders");
		}

		return true;
	}
}