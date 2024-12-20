#pragma once
#include "Epoch/Project/Project.h"
#include "Epoch/Utils/FileSystem.h"
#include "Epoch/Core/Hash.h"
#include "Epoch/Assets/Asset.h"

#include "Epoch/Rendering/Mesh.h" //TODO: Find way not to include here
#include "Epoch/Rendering/Texture.h" //TODO: Find way not to include here
#include "Epoch/Rendering/Environment.h" //TODO: Find way not to include here
#include "Epoch/Scene/Prefab.h" //TODO: Find way not to include here

namespace Epoch
{
	class AssetManager
	{
	public:
		static bool IsAssetHandleValid(AssetHandle aAssetHandle) { return Project::GetAssetManager()->IsAssetHandleValid(aAssetHandle); }
		static bool IsAssetValid(AssetHandle aAssetHandle) { return Project::GetAssetManager()->IsAssetValid(aAssetHandle); }
		static bool IsAssetMissing(AssetHandle aAssetHandle) { return Project::GetAssetManager()->IsAssetMissing(aAssetHandle); }
		static bool IsMemoryAsset(AssetHandle aAssetHandle) { return Project::GetAssetManager()->IsMemoryAsset(aAssetHandle); }

		static bool ReloadData(AssetHandle aAssetHandle) { return Project::GetAssetManager()->ReloadData(aAssetHandle); }
		static void RemoveAsset(AssetHandle aAssetHandle) { Project::GetAssetManager()->RemoveAsset(aAssetHandle); }

		static AssetType GetAssetType(AssetHandle aAssetHandle) { return Project::GetAssetManager()->GetAssetType(aAssetHandle); }

		template<typename T>
		static std::shared_ptr<T> GetAsset(AssetHandle aAssetHandle)
		{
			static_assert(std::is_base_of<Asset, T>::value, "GetAsset only works for types derived from Asset");

			std::shared_ptr<Asset> asset = Project::GetAssetManager()->GetAsset(aAssetHandle);
			return std::dynamic_pointer_cast<T>(asset);
		}

		template<typename T>
		static std::shared_ptr<T> GetAssetAsync(AssetHandle aAssetHandle)
		{
			static_assert(std::is_base_of<Asset, T>::value, "GetAsset only works for types derived from Asset");

			std::shared_ptr<Asset> asset = Project::GetAssetManager()->GetAssetAsync(aAssetHandle);

			if (asset)
			{
				return std::dynamic_pointer_cast<T>(asset);
			}
			
			return nullptr;
		}

		template<typename T>
		static std::unordered_set<AssetHandle> GetAllAssetsWithType()
		{
			static_assert(std::is_base_of<Asset, T>::value, "GetAsset only works for types derived from Asset");

			return Project::GetAssetManager()->GetAllAssetsWithType(T::GetStaticType());
		}

		static const std::unordered_map<AssetHandle, std::shared_ptr<Asset>>& GetLoadedAssets() { return Project::GetAssetManager()->GetLoadedAssets(); }


		template<typename T>
		static AssetHandle AddMemoryOnlyAsset(std::shared_ptr<T> aAsset)
		{
			static_assert(std::is_base_of<Asset, T>::value, "AddMemoryOnlyAsset only works for types derived from Asset");
			aAsset->myHandle = AssetHandle();

			Project::GetAssetManager()->AddMemoryOnlyAsset(aAsset);
			return aAsset->myHandle;
		}

		template<typename T>
		static AssetHandle AddMemoryOnlyAssetWithName(std::shared_ptr<T> aAsset, const std::string& aName)
		{
			static_assert(std::is_base_of<Asset, T>::value, "AddMemoryOnlyAsset only works for types derived from Asset");
			aAsset->myHandle = AssetHandle();

			Project::GetAssetManager()->AddMemoryOnlyAsset(aAsset, aName);
			return aAsset->myHandle;
		}

		template<typename T, typename... TArgs>
		static AssetHandle CreateMemoryOnlyAsset(TArgs&&... aArgs)
		{
			static_assert(std::is_base_of<Asset, T>::value, "CreateMemoryOnlyAsset only works for types derived from Asset");

			std::shared_ptr<T> asset = std::make_shared<T>(std::forward<TArgs>(aArgs)...);
			asset->myHandle = AssetHandle();

			Project::GetAssetManager()->AddMemoryOnlyAsset(asset);
			return asset->myHandle;
		}

		template<typename T, typename... TArgs>
		static AssetHandle CreateMemoryOnlyAssetWithName(const std::string& aName, TArgs&&... aArgs)
		{
			static_assert(std::is_base_of<Asset, T>::value, "CreateMemoryOnlyAsset only works for types derived from Asset");

			std::shared_ptr<T> asset = std::make_shared<T>(std::forward<TArgs>(aArgs)...);
			asset->myHandle = Hash::GenerateFNVHash(aName);

			Project::GetAssetManager()->AddMemoryOnlyAsset(asset, aName);
			return asset->myHandle;
		}
	};
}
