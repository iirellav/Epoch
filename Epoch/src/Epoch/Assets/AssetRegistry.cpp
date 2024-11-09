#include "epch.h"
#include "AssetRegistry.h"

namespace Epoch
{
	//static std::mutex staticAssetRegistryMutex;

	AssetMetadata& AssetRegistry::operator[](const AssetHandle aHandle)
	{
		//std::scoped_lock<std::mutex> lock(staticAssetRegistryMutex);

		return myAssetRegistry[aHandle];
	}

	AssetMetadata& AssetRegistry::Get(const AssetHandle aHandle)
	{
		//std::scoped_lock<std::mutex> lock(staticAssetRegistryMutex);

		EPOCH_ASSERT(myAssetRegistry.find(aHandle) != myAssetRegistry.end(), "No asset with handle exists in registry!");
		return myAssetRegistry.at(aHandle);
	}

	const AssetMetadata& AssetRegistry::Get(const AssetHandle aHandle) const
	{
		//std::scoped_lock<std::mutex> lock(staticAssetRegistryMutex);

		return myAssetRegistry.at(aHandle);
	}

	bool AssetRegistry::Contains(const AssetHandle aHandle) const
	{
		//std::scoped_lock<std::mutex> lock(staticAssetRegistryMutex);

		return myAssetRegistry.find(aHandle) != myAssetRegistry.end();
	}

	size_t AssetRegistry::Remove(const AssetHandle aHandle)
	{
		//std::scoped_lock<std::mutex> lock(staticAssetRegistryMutex);

		return myAssetRegistry.erase(aHandle);
	}

	void AssetRegistry::Clear()
	{
		//std::scoped_lock<std::mutex> lock(staticAssetRegistryMutex);

		myAssetRegistry.clear();
	}
}
