#include <string>
#include <Epoch/Core/Application.h>
#include <Epoch/Core/EntryPoint.h>
#include <Epoch/Utils/FileSystem.h>
#include "EditorLayer.h"

#include "Epoch/Core/Base.h"
#include "Epoch/Debug/Instrumentor.h"

namespace Epoch
{
	int Main(int argc, char** argv)
	{
		argc; argv;
		
		InitializeCore();

		EPOCH_PROFILE_BEGIN_SESSION("Startup", "Profiling_Results/EpochProfile-Startup.json", false);

		ApplicationSpecification applicationSpecification;
		applicationSpecification.isRuntime = false;
		applicationSpecification.name = "Aeon";
		applicationSpecification.startMaximized = true;
		applicationSpecification.cacheDirectory = "Resources/cache";
		
		applicationSpecification.scriptEngineConfig.coreAssemblyPath = "Resources/Scripts/Epoch-ScriptCore.dll";

		auto app = new Application(applicationSpecification);

		//Update the EPOCH_DIR environment variable every time we launch
		{
			EPOCH_PROFILE_SCOPE("Aeon: Update environment variable");
			std::filesystem::path workingDirectory = std::filesystem::current_path();
			if (workingDirectory.stem().string() == "Aeon")
			{
				workingDirectory = workingDirectory.parent_path();
			}
			FileSystem::SetEnvironmentVariable("EPOCH_DIR", workingDirectory.string());
		}

		app->PushLayer(new EditorLayer());

		EPOCH_PROFILE_END_SESSION();

		EPOCH_PROFILE_BEGIN_SESSION("Runtime", "Profiling_Results/EpochProfile-Runtime.json", true);
		app->Run();
		EPOCH_PROFILE_END_SESSION();

		EPOCH_PROFILE_BEGIN_SESSION("Shutdown", "Profiling_Results/EpochProfile-Shutdown.json", false);
		delete app;
		app = nullptr;
		EPOCH_PROFILE_END_SESSION();

		ShutdownCore();

		return 0;
	}

	//class Aeon : public Application
	//{
	//public:
	//	Aeon(ApplicationSpecification& aApplicationSpecification) : Application(aApplicationSpecification)
	//	{
	//		// Update the EPOCH_DIR environment variable every time we launch
	//		{
	//			EPOCH_PROFILE_SCOPE("Aeon: Update environment variable");
	//			std::filesystem::path workingDirectory = std::filesystem::current_path();
	//
	//			if (workingDirectory.stem().string() == "Aeon")
	//			{
	//				workingDirectory = workingDirectory.parent_path();
	//			}
	//
	//			FileSystem::SetEnvironmentVariable("EPOCH_DIR", workingDirectory.string());
	//		}
	//
	//		PushLayer(new EditorLayer());
	//	}
	//};
	//
	//Application* CreateApplication()
	//{
	//	ApplicationSpecification applicationSpecification;
	//	applicationSpecification.isRuntime = false;
	//	applicationSpecification.name = "Aeon";
	//	applicationSpecification.startMaximized = true;
	//	applicationSpecification.cacheDirectory = "Resources/cache";
	//
	//	applicationSpecification.scriptEngineConfig.coreAssemblyPath = "Resources/Scripts/Epoch-ScriptCore.dll";
	//
	//	return new Aeon(applicationSpecification);
	//}
}

#if _DIST && PLATFORM_WINDOWS

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	return Epoch::Main(__argc, __argv);
}

#else

int main(int argc, char** argv)
{
	return Epoch::Main(argc, argv);
}

#endif
