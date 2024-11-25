#pragma once
#include <map>
#include "AssetMetadata.h"

namespace Epoch
{
	class AssetRegistry
	{
	public:
		AssetRegistry() = default;
		~AssetRegistry() = default;

		AssetMetadata& operator[](const AssetHandle aHandle);
		AssetMetadata& Get(const AssetHandle aHandle);
		const AssetMetadata& Get(const AssetHandle aHandle) const;

		size_t Count() const { return myAssetRegistry.size(); }
		bool Contains(const AssetHandle aHandle) const;
		size_t Remove(const AssetHandle aHandle);
		void Clear();

		auto begin() { return myAssetRegistry.begin(); }
		auto end() { return myAssetRegistry.end(); }
		auto begin() const { return myAssetRegistry.cbegin(); }
		auto end() const { return myAssetRegistry.cend(); }

	private:
		std::map<AssetHandle, AssetMetadata> myAssetRegistry;
	};
}
