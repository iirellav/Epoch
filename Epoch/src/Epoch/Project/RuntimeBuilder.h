#pragma once
#include <filesystem>

namespace Epoch
{
	class RuntimeBuilder
	{
	public:
		static bool Build(const std::filesystem::path& aBuildLocation);

	private:
		static void SetResources();
		static std::pair<std::string, std::string> SetIcon();

	private:
		static inline std::filesystem::path myBuildLocation;
	};
}
