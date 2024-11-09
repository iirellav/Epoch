#pragma once
//#include "Epoch/Core/Base.h"
//#include "Epoch/Core/Application.h"
//#include "Epoch/Debug/Instrumentor.h"
//
//extern Epoch::Application* Epoch::CreateApplication();
//
//namespace Epoch
//{
//	int Main(int argc, char** argv)
//	{
//		argc; argv;
//		
//		InitializeCore();
//
//		EPOCH_PROFILE_BEGIN_SESSION("Startup", "Profiling_Results/EpochProfile-Startup.json", false);
//		auto app = Epoch::CreateApplication();
//		EPOCH_PROFILE_END_SESSION();
//
//		EPOCH_PROFILE_BEGIN_SESSION("Runtime", "Profiling_Results/EpochProfile-Runtime.json", true);
//		app->Run();
//		EPOCH_PROFILE_END_SESSION();
//
//		EPOCH_PROFILE_BEGIN_SESSION("Shutdown", "Profiling_Results/EpochProfile-Shutdown.json", false);
//		delete app;
//		app = nullptr;
//		EPOCH_PROFILE_END_SESSION();
//
//		ShutdownCore();
//
//		return 0;
//	}
//}
//
//
//#if _DIST && PLATFORM_WINDOWS
//
//int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
//{
//	return Epoch::Main(__argc, __argv);
//}
//
//#else
//
//int main(int argc, char** argv)
//{
//	return Epoch::Main(argc, argv);
//}
//
//#endif
