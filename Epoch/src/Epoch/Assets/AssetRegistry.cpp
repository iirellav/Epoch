#include "epch.h"
#include "AssetRegistry.h"

namespace Epoch
{
	AssetMetadata& AssetRegistry::operator[](const AssetHandle aHandle)
	{
		return myAssetRegistry[aHandle];
	}

	AssetMetadata& AssetRegistry::Get(const AssetHandle aHandle)
	{
		EPOCH_ASSERT(myAssetRegistry.find(aHandle) != myAssetRegistry.end(), "No asset with handle exists in registry!");
		return myAssetRegistry.at(aHandle);
	}

	const AssetMetadata& AssetRegistry::Get(const AssetHandle aHandle) const
	{
		return myAssetRegistry.at(aHandle);
	}

	bool AssetRegistry::Contains(const AssetHandle aHandle) const
	{
		return myAssetRegistry.find(aHandle) != myAssetRegistry.end();
	}

	size_t AssetRegistry::Remove(const AssetHandle aHandle)
	{
		return myAssetRegistry.erase(aHandle);
	}

	void AssetRegistry::Clear()
	{
		myAssetRegistry.clear();
	}
}
