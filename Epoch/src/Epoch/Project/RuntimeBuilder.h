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
		static void SetIcon(std::string& outIcoConvertCmd, std::string& outDeleteIcoCmd);

	private:
		static inline std::filesystem::path myBuildLocation;
	};
}
