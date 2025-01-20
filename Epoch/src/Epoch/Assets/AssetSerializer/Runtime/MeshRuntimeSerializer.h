#pragma once
#include "Epoch/Assets/AssetSerializer/AssetSerializer.h"
#include "Epoch/Assets/AssetPack/AssetPack.h"
#include "Epoch/Assets/AssetPack//AssetPackFile.h"
#include "Epoch/Serialization/FileStream.h"

namespace Epoch
{
	class MeshRuntimeSerializer
	{
	public:
		bool SerializeToAssetPack(AssetHandle aHandle, FileStreamWriter& aStream, AssetSerializationInfo& outInfo);
		std::shared_ptr<Asset> DeserializeFromAssetPack(FileStreamReader& aStream, const AssetPackFile::AssetInfo& aAssetInfo);
	};
}
