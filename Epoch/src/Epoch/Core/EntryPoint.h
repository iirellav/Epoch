#pragma once
#include "Epoch/Core/Base.h"
#include "Epoch/Core/Application.h"
#include "Epoch/Debug/Profiler.h"

extern Epoch::Application* Epoch::CreateApplication(int aArgc, char** aArgv);

namespace Epoch
{
	int Main(int argc, char** argv)
	{
		argc; argv;
		
		InitializeCore();

		auto app = Epoch::CreateApplication(argc, argv);
		app->Run();
		delete app;
		app = nullptr;

		ShutdownCore();

		return 0;
	}
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
