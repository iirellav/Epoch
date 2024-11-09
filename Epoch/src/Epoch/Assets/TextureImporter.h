#pragma once
#include <filesystem>
#include "Epoch/Core/Buffer.h"
#include "Epoch/Rendering/Texture.h"

namespace Epoch
{
	class TextureImporter
	{
	public:
		static Buffer ToBufferFromFile(const std::filesystem::path& aPath, TextureFormat& outFormat, uint32_t& outWidth, uint32_t& outHeight);
	};
}
