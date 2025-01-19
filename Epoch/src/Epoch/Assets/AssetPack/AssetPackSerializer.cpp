#include "epch.h"
#include "AssetPackSerializer.h"
#include "Epoch/Serialization/FileStream.h"
#include "Epoch/Assets/AssetImporter.h"

namespace Epoch
{
	void AssetPackSerializer::Serialize(const std::filesystem::path& aPath, AssetPackFile& aFile, Buffer aAppBinary, std::atomic<float>& aProgress)
	{
		LOG_INFO("Serializing AssetPack to {}", aPath.string());

		FileStreamWriter serializer(aPath);
		serializer.WriteRaw<AssetPackFile::FileHeader>(aFile.header);

		// ===============
		// Write index
		// ===============
		// Write dummy data for index (come back later to fill in)
		uint64_t indexTablePos = serializer.GetStreamPosition();
		uint64_t indexTableSize = CalculateIndexTableSize(aFile);
		serializer.WriteZero(indexTableSize);

		std::unordered_map<AssetHandle, AssetSerializationInfo> serializedAssets;

		float progressIncrement = 0.4f / (float)aFile.indexTable.scenes.size();

		// Write app binary data
		aFile.indexTable.packedAppBinaryOffset = serializer.GetStreamPosition();
		serializer.WriteBuffer(aAppBinary);
		aFile.indexTable.packedAppBinarySize = serializer.GetStreamPosition() - aFile.indexTable.packedAppBinaryOffset;
		aAppBinary.Release();

		// Write asset data
		for (auto& [sceneHandle, sceneInfo] : aFile.indexTable.scenes)
		{
			// Serialize Scene
			AssetSerializationInfo serializationInfo;
			AssetImporter::SerializeToAssetPack(sceneHandle, serializer, serializationInfo);
			aFile.indexTable.scenes[sceneHandle].packedOffset = serializationInfo.offset;
			aFile.indexTable.scenes[sceneHandle].packedSize = serializationInfo.size;

			// Serialize Assets
			for (auto& [assetHandle, assetInfo] : sceneInfo.assets)
			{
				if (serializedAssets.find(assetHandle) != serializedAssets.end())
				{
					// Has already been serialized
					serializationInfo = serializedAssets.at(assetHandle);
					aFile.indexTable.scenes[sceneHandle].assets[assetHandle].packedOffset = serializationInfo.offset;
					aFile.indexTable.scenes[sceneHandle].assets[assetHandle].packedSize = serializationInfo.size;
				}
				else
				{
					// Serialize asset
					if (!AssetImporter::SerializeToAssetPack(assetHandle, serializer, serializationInfo))
					{
						//CONSOLE_LOG_ERROR("Failed to serialize asset '{}' to asset pack!", (uint64_t)assetHandle);
						EPOCH_ASSERT("Failed to serialize asset '{}' to asset pack!", (uint64_t)assetHandle);
					}

					aFile.indexTable.scenes[sceneHandle].assets[assetHandle].packedOffset = serializationInfo.offset;
					aFile.indexTable.scenes[sceneHandle].assets[assetHandle].packedSize = serializationInfo.size;
					serializedAssets[assetHandle] = serializationInfo;
				}
			}

			aProgress += progressIncrement;
		}

		LOG_INFO("Serialized {} assets into AssetPack", serializedAssets.size());

		// Fill index table
		serializer.SetStreamPosition(indexTablePos);
		serializer.WriteRaw<uint64_t>(aFile.indexTable.packedAppBinaryOffset);
		serializer.WriteRaw<uint64_t>(aFile.indexTable.packedAppBinarySize);

		serializer.WriteRaw<uint32_t>((uint32_t)aFile.indexTable.scenes.size()); // Write scene map size
		for (auto& [sceneHandle, sceneInfo] : aFile.indexTable.scenes)
		{
			serializer.WriteRaw<uint64_t>(sceneHandle);
			serializer.WriteRaw<uint64_t>(sceneInfo.packedOffset);
			serializer.WriteRaw<uint64_t>(sceneInfo.packedSize);

			serializer.WriteMap(aFile.indexTable.scenes[sceneHandle].assets);
		}
	}

	bool AssetPackSerializer::DeserializeIndex(const std::filesystem::path& aPath, AssetPackFile& aFile)
	{
		return false;
	}

	uint64_t AssetPackSerializer::CalculateIndexTableSize(const AssetPackFile& aFile)
	{
		uint64_t appBinaryInfoSize = sizeof(uint64_t) * 2; //packedAppBinaryOffset & packedAppBinarySize
		uint64_t sceneMapSize = sizeof(uint32_t) + (sizeof(AssetHandle) + sizeof(uint64_t) * 2) * aFile.indexTable.scenes.size(); //scene count + (scene handle + scene info) * scene count
		uint64_t assetMapSize = 0;
		for (const auto& [sceneHandle, sceneInfo] : aFile.indexTable.scenes)
		{
			assetMapSize += sizeof(uint32_t) + (sizeof(AssetHandle) + sizeof(AssetPackFile::AssetInfo)) * sceneInfo.assets.size(); //asset count + (asset handle + asset info) * asset count
		}

		return appBinaryInfoSize + sceneMapSize + assetMapSize;
	}
}
