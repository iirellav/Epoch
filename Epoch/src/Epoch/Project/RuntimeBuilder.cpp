#include "epch.h"
#include "RuntimeBuilder.h"
#include <ShlObj.h>
#include "Project.h"
#include "ProjectSerializer.h"
#include "Epoch/Utils/FileSystem.h"
#include "Epoch/Assets/AssetPack/AssetPack.h"
#include "Epoch/Assets/AssetManager.h"
#include "Epoch/Rendering/ShaderPack.h"
#include "Epoch/Rendering/Renderer.h"
#include "Epoch/Debug/Timer.h"
#include "Epoch/Editor/EditorSettings.h"

namespace Epoch
{
	constexpr std::string IconPath = "icon.ico";

	bool RuntimeBuilder::Build(const std::filesystem::path& aBuildLocation)
	{
		EPOCH_PROFILE_FUNC();

		myBuildLocation = aBuildLocation;

		Timer buildTimer;

		const ProjectConfig& configs = Project::GetActive()->GetConfig();

		const auto projDir = Project::GetProjectDirectory();
		const auto projFilePath = Project::GetProjectPath();

		std::filesystem::path epochDir = std::filesystem::current_path();
		
		FileSystem::CopyContent(epochDir / "Resources/Runtime", myBuildLocation);

		SetResources();

		ProjectSerializer projectSerializer(Project::GetActive());
		projectSerializer.SerializeRuntime(myBuildLocation / "Project.eproj");

		FileSystem::CopyFile("Resources/Scripts/Epoch-ScriptCore.dll", myBuildLocation);

		FileSystem::CreateDirectory(myBuildLocation / "Assets");

		AssetPack::CreateFromActiveProject(myBuildLocation / "Assets/AssetPack.eap");
		ShaderPack::CreateFromLibrary(Renderer::GetShaderLibrary(), myBuildLocation / "Assets/ShaderPack.esp");

		CONSOLE_LOG_INFO("Build took {}s to complete", buildTimer.Elapsed());

		return true;
	}

	void RuntimeBuilder::SetResources()
	{
		if (!FileSystem::Exists("ExternalTools/rcedit.exe"))
		{
			return;
		}
		const std::string rcEditPath = std::filesystem::absolute("ExternalTools/rcedit.exe").string();

		const ProjectConfig& configs = Project::GetActive()->GetConfig();

		std::string icoConvertCmd;
		std::string deleteIcoCmd;
		SetIcon(icoConvertCmd, deleteIcoCmd);

		std::ifstream stream(myBuildLocation / "SetResources.bat");
		EPOCH_ASSERT(stream.is_open(), "Could not open project file!");
		std::stringstream ss;
		ss << stream.rdbuf();
		stream.close();

		std::string str = ss.str();
		CU::ReplaceToken(str, "$ICO_CONVERT_CMD$", icoConvertCmd);
		CU::ReplaceToken(str, "$DELETE_ICO_CMD$", deleteIcoCmd);
		CU::ReplaceToken(str, "$RCEDIT$", rcEditPath);
		CU::ReplaceToken(str, "$ICON_PATH$", IconPath);
		CU::ReplaceToken(str, "$PRUDUCT_NAME$", configs.productName);

		std::ofstream ostream(myBuildLocation / "SetResources.bat");
		ostream << str;
		ostream.close();
		
		WinExec((myBuildLocation / "SetResources.bat").string().c_str(), SW_HIDE);
	}

	void RuntimeBuilder::SetIcon(std::string& outIcoConvertCmd, std::string& outDeleteIcoCmd)
	{
		const ProjectConfig& configs = Project::GetActive()->GetConfig();

		if (configs.appIcon == 0 || !FileSystem::Exists("ExternalTools/magick.exe"))
		{
			return;
		}

		const auto metadata = Project::GetEditorAssetManager()->GetMetadata(configs.appIcon);
		if (metadata.filePath.extension() != ".png")
		{
			CONSOLE_LOG_ERROR("App icon needs to be of type 'png'");
			return;
		}

		auto icon = AssetManager::GetAsset<Texture2D>(configs.appIcon);

		if (icon->GetWidth() != icon->GetHeight())
		{
			CONSOLE_LOG_ERROR("App icon needs to be a square");
			return;
		}

		std::string icoConvertCmd = "";
		std::string deleteIcoCmd = "";
		const std::filesystem::path fullIconPath = Project::GetEditorAssetManager()->GetFileSystemPath(configs.appIcon);
		if (FileSystem::Exists(fullIconPath))
		{
			const std::string magicPath = std::filesystem::absolute("ExternalTools/magick.exe").string();
			outIcoConvertCmd = std::format("call \"{}\" \"{}\" -define icon:auto-resize=16,24,32,48,64,72,96,128,256 \"{}\"", magicPath, fullIconPath.string(), IconPath);
			outDeleteIcoCmd = std::format("call del {}", IconPath);
		}
	}
}