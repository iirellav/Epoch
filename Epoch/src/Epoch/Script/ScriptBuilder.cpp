#include "epch.h"
#include "ScriptBuilder.h"
#include <ShlObj.h>
#include <cstdlib>

namespace Epoch
{
	void ScriptBuilder::BuildCSProject(const std::filesystem::path& aFilepath)
	{
		EPOCH_PROFILE_FUNC();

		TCHAR programFilesFilePath[MAX_PATH];
		SHGetSpecialFolderPath(0, programFilesFilePath, CSIDL_PROGRAM_FILES, FALSE);
		std::filesystem::path msBuildPath = std::filesystem::path(programFilesFilePath) / "Microsoft Visual Studio" / "2022" / "Community" / "Msbuild" / "Current" / "Bin" / "MSBuild.exe";

		const std::string projFileDir = aFilepath.parent_path().string();
		std::string command = fmt::format("cd \"{}\" && \"{}\" \"{}\" -property:Configuration=Debug", projFileDir, msBuildPath.string(), aFilepath.string());
		//std::system(command.c_str());
		WinExec(command.c_str(), SW_HIDE);
	}

	void ScriptBuilder::BuildScriptAssembly()
	{
		EPOCH_PROFILE_FUNC();

		const auto projectAssemblyFile = Project::GetProjectDirectory() / (Project::GetProjectName() + ".csproj");
		if (!FileSystem::Exists(projectAssemblyFile))
		{
			return;
		}
		BuildCSProject(projectAssemblyFile);
	}

	void ScriptBuilder::RegenerateScriptSolution(const std::filesystem::path& aProjectDirectory)
	{
		EPOCH_PROFILE_FUNC();

		std::string batchFilePath = aProjectDirectory.string();
		std::replace(batchFilePath.begin(), batchFilePath.end(), '/', '\\'); // Only windows
		batchFilePath += "\\CreateScriptProject.bat";

		if (!FileSystem::Exists(batchFilePath))
		{
			LOG_WARNING("File not found ''!", batchFilePath);
		}
		
		batchFilePath = "\"" + batchFilePath + "\"";
		WinExec(batchFilePath.c_str(), SW_HIDE);
		//std::system(batchFilePath.c_str());
	}
}
