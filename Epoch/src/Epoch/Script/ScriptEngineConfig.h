#pragma once
#include <filesystem>

namespace Epoch
{
	struct ScriptEngineConfig
	{
		std::filesystem::path coreAssemblyPath;
		//bool enableDebugging;
		//bool enableProfiling;
	};
}