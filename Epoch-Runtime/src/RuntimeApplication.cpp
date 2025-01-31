#include <string>
#include <filesystem>
#include <Epoch/Core/Application.h>
#include <Epoch/Core/EntryPoint.h>
#include "RuntimeLayer.h"

namespace Epoch
{
	class Aeon : public Application
	{
	public:
		Aeon(ApplicationSpecification& aApplicationSpecification, std::string_view aProjectPath) : Application(aApplicationSpecification)
		{
			PushLayer(new RuntimeLayer(aProjectPath));
		}
	};

	Application* CreateApplication(int aArgc, char** aArgv)
	{
		ApplicationSpecification applicationSpecification;
		applicationSpecification.isRuntime = true;
		//applicationSpecification.name = "Epoch-Runtime";
		applicationSpecification.name = std::filesystem::path(aArgv[0]).stem().string();
		//applicationSpecification.startFullscreen = true;
		applicationSpecification.enableImGui = false;
		applicationSpecification.vSync = false;
		applicationSpecification.cacheDirectory = "cache";
		applicationSpecification.scriptEngineConfig.coreAssemblyPath = "Epoch-ScriptCore.dll";
		applicationSpecification.rendererConfig.shaderPackPath = "Assets/ShaderPack.esp";

		std::string projectPath = "Project.eproj";

		return new Aeon(applicationSpecification, projectPath);
	}
}
