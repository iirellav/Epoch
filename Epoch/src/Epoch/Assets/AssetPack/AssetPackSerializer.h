#pragma once
#include <filesystem>
#include "AssetPackFile.h"
#include "Epoch/Core/Buffer.h"

namespace Epoch
{
	class AssetPackSerializer
	{
	public:
		void Serialize(const std::filesystem::path& aPath, AssetPackFile& aFile, Buffer aAppBinary);
		bool DeserializeIndex(const std::filesystem::path& aPath, AssetPackFile& aFile);

	private:
		uint64_t CalculateIndexTableSize(const AssetPackFile& aFile);
	};
}
