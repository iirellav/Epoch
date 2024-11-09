#include <string>
//#include <Epoch/Core/Application.h>
//#include <Epoch/Core/EntryPoint.h>

#include <Epoch/Core/Base.h>
#include <Epoch/Core/Application.h>
#include <Epoch/Debug/Instrumentor.h>

#include "RuntimeLayer.h"

namespace Epoch
{
	int Main(int argc, char** argv)
	{
		argc; argv;
		
		InitializeCore();

		EPOCH_PROFILE_BEGIN_SESSION("Startup", "Profiling_Results/EpochProfile-Startup.json", false);

		ApplicationSpecification applicationSpecification;
		applicationSpecification.isRuntime = true;
		applicationSpecification.name = "Epoch-Runtime";
		applicationSpecification.startFullscreen = true;
		applicationSpecification.enableImGui = false;
		applicationSpecification.vSync = false;
		applicationSpecification.cacheDirectory = "Resources/cache";
		applicationSpecification.scriptEngineConfig.coreAssemblyPath = "Scripts/Binaries/Epoch-ScriptCore.dll";
		
		std::string projectPath = "Project.eproj";

		auto app = new Application(applicationSpecification);
		app->PushLayer(new RuntimeLayer(projectPath));

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
	//	Aeon(ApplicationSpecification& aApplicationSpecification, std::string_view aProjectPath) : Application(aApplicationSpecification)
	//	{
	//		PushLayer(new RuntimeLayer(aProjectPath));
	//	}
	//};
	//
	//Application* CreateApplication()
	//{
	//	ApplicationSpecification applicationSpecification;
	//	applicationSpecification.isRuntime = true;
	//	applicationSpecification.name = "Epoch-Runtime";
	//	applicationSpecification.startFullscreen = true;
	//	applicationSpecification.enableImGui = false;
	//	applicationSpecification.vSync = false;
	//	applicationSpecification.cacheDirectory = "Resources/cache";
	//	applicationSpecification.scriptEngineConfig.coreAssemblyPath = "Scripts/Binaries/Epoch-ScriptCore.dll";
	//
	//	std::string projectPath = "Project.eproj";
	//
	//	return new Aeon(applicationSpecification, projectPath);
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
