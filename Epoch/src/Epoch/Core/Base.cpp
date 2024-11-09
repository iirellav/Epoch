#include "epch.h"
#include "Base.h"
#include "Epoch/Debug/Log.h"

namespace Epoch
{
	void InitializeCore()
	{
		Log::Init();
	}

	void ShutdownCore()
	{
		Log::Shutdown();
	}
}