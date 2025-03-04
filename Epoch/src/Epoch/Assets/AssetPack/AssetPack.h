#pragma once
#include <memory>
#include <filesystem>
#include "AssetPackFile.h"

namespace Epoch
{
	class Scene;
	class Asset;

	class AssetPack
	{
	public:
		enum class BuildError
		{
			None,
			NoStartScene
		};

	public:
		static bool CreateFromActiveProject(std::filesystem::path aDestination = "");
		static std::shared_ptr<AssetPack> Load(const std::filesystem::path& aPath);

		std::shared_ptr<Scene> LoadScene(AssetHandle aSceneHandle);
		std::shared_ptr<Asset> LoadAsset(AssetHandle aSceneHandle, AssetHandle aAssetHandle);

		bool IsAssetHandleValid(AssetHandle assetHandle) const;

		Buffer ReadAppBinary();

		static const char* GetLastErrorMessage();

	private:
		std::filesystem::path myPath;
		AssetPackFile myFile;

		std::unordered_set<AssetHandle> myAssetHandleIndex;

		static inline BuildError myLastBuildError;
	};
}
