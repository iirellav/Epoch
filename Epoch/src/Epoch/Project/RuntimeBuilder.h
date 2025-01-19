#pragma once
#include <filesystem>

namespace Epoch
{
	class RuntimeBuilder
	{
	public:
		static bool Build(std::filesystem::path aBuildLocation);
	};
}
