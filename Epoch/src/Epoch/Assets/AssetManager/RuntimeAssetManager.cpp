#include "epch.h"
#include "RuntimeAssetManager.h"
#include "Epoch/Assets/AssetImporter.h"
#include "Epoch/Assets/AssetManager.h"
#include "Epoch/Core/Application.h"
#include "Epoch/Rendering/MeshFactory.h"

namespace Epoch
{
	RuntimeAssetManager::RuntimeAssetManager()
	{
		AssetImporter::Init();
	}

	void RuntimeAssetManager::LoadBuiltInAssets()
	{
		MeshFactory::CreateCube();
		MeshFactory::CreateSphere();
		MeshFactory::CreateQuad();
		MeshFactory::CreatePlane();

		AssetManager::CreateMemoryOnlyAssetWithName<Material>("Default-Material");
	}

	AssetType RuntimeAssetManager::GetAssetType(AssetHandle aHandle)
	{
		std::shared_ptr<Asset> asset = GetAsset(aHandle);
		if (!asset)
		{
			return AssetType::None;
		}

		return asset->GetAssetType();
	}
	
	std::shared_ptr<Asset> RuntimeAssetManager::GetAsset(AssetHandle aHandle)
	{
		if (IsMemoryAsset(aHandle))
		{
			return myMemoryAssets[aHandle];
		}

		std::shared_ptr<Asset> asset = nullptr;
		bool isLoaded = IsAssetLoaded(aHandle);
		if (isLoaded)
		{
			asset = myLoadedAssets[aHandle];
		}
		else
		{
			// Needs load
			asset = myAssetPack->LoadAsset(myActiveScene, aHandle);
			if (!asset)
			{
				return nullptr;
			}

			myLoadedAssets[aHandle] = asset;
		}

		return asset;
	}
	
	std::shared_ptr<Asset> RuntimeAssetManager::GetAssetAsync(AssetHandle aHandle)
	{
		return GetAsset(aHandle);
	}
	
	void RuntimeAssetManager::AddMemoryOnlyAsset(std::shared_ptr<Asset> aAsset)
	{
		myMemoryAssets[aAsset->GetHandle()] = aAsset;
	}
	
	void RuntimeAssetManager::AddMemoryOnlyAsset(std::shared_ptr<Asset> aAsset, const std::string& aName)
	{
		AddMemoryOnlyAsset(aAsset);
	}
	
	bool RuntimeAssetManager::ReloadData(AssetHandle aAssetHandle)
	{
		std::shared_ptr<Asset> asset = myAssetPack->LoadAsset(myActiveScene, aAssetHandle);
		if (asset)
		{
			myLoadedAssets[aAssetHandle] = asset;
		}

		return asset != nullptr;
	}
	
	void RuntimeAssetManager::RemoveAsset(AssetHandle aHandle)
	{
		if (myLoadedAssets.contains(aHandle))
		{
			myLoadedAssets.erase(aHandle);
			
		}
		else if (myMemoryAssets.contains(aHandle))
		{
			myMemoryAssets.erase(aHandle);
		}
	}
	
	bool RuntimeAssetManager::IsMemoryAsset(AssetHandle aHandle)
	{
		return myMemoryAssets.contains(aHandle);
	}
	
	bool RuntimeAssetManager::IsAssetHandleValid(AssetHandle aHandle)
	{
		if (aHandle == 0)
		{
			return false;
		}

		return IsMemoryAsset(aHandle) || (myAssetPack && myAssetPack->IsAssetHandleValid(aHandle));
	}
	
	bool RuntimeAssetManager::IsAssetLoaded(AssetHandle aHandle)
	{
		return myLoadedAssets.contains(aHandle);
	}
		
	std::unordered_set<AssetHandle> RuntimeAssetManager::GetAllAssetsWithType(AssetType aType)
	{
		std::unordered_set<AssetHandle> result;
		EPOCH_ASSERT(false, "Not implemented");
		return result;
	}
	
	const std::unordered_map<AssetHandle, std::shared_ptr<Asset>>& RuntimeAssetManager::GetLoadedAssets()
	{
		return myLoadedAssets;
	}

	std::shared_ptr<Scene> RuntimeAssetManager::LoadScene(AssetHandle aHandle)
	{
		std::shared_ptr<Scene> scene = myAssetPack->LoadScene(aHandle);
		if (scene)
		{
			myActiveScene = aHandle;
		}

		return scene;
	}
}
