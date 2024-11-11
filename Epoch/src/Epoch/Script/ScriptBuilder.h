#pragma once
#include "Epoch/Project/Project.h"

namespace Epoch
{
	class ScriptBuilder
	{
	public:
		static void BuildCSProject(const std::filesystem::path& aFilepath);
		static void BuildScriptAssembly();
		static void RegenerateScriptSolution(const std::filesystem::path& aProjectDirectory);
	};
}
