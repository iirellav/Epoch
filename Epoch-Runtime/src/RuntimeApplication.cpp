#include <string>
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

	Application* CreateApplication()
	{
		ApplicationSpecification applicationSpecification;
		applicationSpecification.isRuntime = true;
		applicationSpecification.name = "Epoch-Runtime";
		applicationSpecification.startFullscreen = true;
		applicationSpecification.enableImGui = false;
		applicationSpecification.vSync = false;
		applicationSpecification.cacheDirectory = "cache";
		applicationSpecification.scriptEngineConfig.coreAssemblyPath = "Epoch-ScriptCore.dll";
		applicationSpecification.rendererConfig.shaderPackPath = "Assets/ShaderPack.esp";

		std::string projectPath = "Project.eproj";

		return new Aeon(applicationSpecification, projectPath);
	}
}
