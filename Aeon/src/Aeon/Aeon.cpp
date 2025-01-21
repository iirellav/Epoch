#include <string>
#include <Epoch/Core/Application.h>
#include <Epoch/Core/EntryPoint.h>
#include <Epoch/Utils/FileSystem.h>
#include "EditorLayer.h"

namespace Epoch
{
	class Aeon : public Application
	{
	public:
		Aeon(ApplicationSpecification& aApplicationSpecification) : Application(aApplicationSpecification)
		{
			// Update the EPOCH_DIR environment variable every time we launch
			{
				EPOCH_PROFILE_SCOPE("Aeon: Update environment variable");
				std::filesystem::path workingDirectory = std::filesystem::current_path();
				FileSystem::SetEnvironmentVariable("EPOCH_DIR", workingDirectory.string());
			}
	
			PushLayer(new EditorLayer());
		}
	};
	
	Application* CreateApplication()
	{
		ApplicationSpecification applicationSpecification;
		applicationSpecification.isRuntime = false;
		applicationSpecification.name = "Aeon";
		applicationSpecification.iconPath = "Resources/Icons/Epoch_Icon.png";
		applicationSpecification.startMaximized = true;
		applicationSpecification.cacheDirectory = "Resources/cache";
	
		applicationSpecification.scriptEngineConfig.coreAssemblyPath = "Resources/Scripts/Epoch-ScriptCore.dll";
	
		return new Aeon(applicationSpecification);
	}
}