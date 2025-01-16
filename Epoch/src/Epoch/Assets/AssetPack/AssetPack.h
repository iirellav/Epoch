#pragma once
#include <memory>
#include <filesystem>
#include "AssetPackFile.h"

namespace Epoch
{
	class AssetPack
	{
	public:
		enum class BuildError
		{
			None,
			NoStartScene
		};

	public:
		static bool CreateFromActiveProject(std::atomic<float>& aProgress);
		static std::shared_ptr<AssetPack> LoadActiveProject();

		static const char* GetLastErrorMessage();

	private:
		std::filesystem::path myPath;
		AssetPackFile myFile;

		static inline BuildError myLastBuildError;
	};
}
