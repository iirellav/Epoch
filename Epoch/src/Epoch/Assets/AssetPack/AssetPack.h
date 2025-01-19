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
		static bool CreateFromActiveProject(std::atomic<float>& aProgress, std::filesystem::path aDestination = "");
		static std::shared_ptr<AssetPack> Load(const std::filesystem::path& aPath);

		static const char* GetLastErrorMessage();

		Buffer ReadAppBinary();

	private:
		std::filesystem::path myPath;
		AssetPackFile myFile;

		std::unordered_set<AssetHandle> myAssetHandleIndex;

		static inline BuildError myLastBuildError;
	};
}
