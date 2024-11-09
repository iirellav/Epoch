#include "epch.h"
#include "TextureImporter.h"

#pragma warning(push, 0)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#pragma warning(pop)

namespace Epoch
{
	Buffer TextureImporter::ToBufferFromFile(const std::filesystem::path& aPath, TextureFormat& outFormat, uint32_t& outWidth, uint32_t& outHeight)
	{
		EPOCH_PROFILE_FUNC();

		Buffer imageBuffer;
		std::string pathString = aPath.string();
		int width, height, channels;
		TextureFormat format;

		if (stbi_is_hdr(pathString.c_str()))
		{
			imageBuffer.data = (byte*)stbi_loadf(pathString.c_str(), &width, &height, &channels, 4);
			imageBuffer.size = width * height * 4 * sizeof(float);
			format = TextureFormat::RGBA32F;
		}
		else
		{
			imageBuffer.data = stbi_load(pathString.c_str(), &width, &height, &channels, 4);
			imageBuffer.size = width * height * 4;
			format = TextureFormat::RGBA;
		}

		if (!imageBuffer.data) return {};

		outWidth = width;
		outHeight = height;
		outFormat = format;
		return imageBuffer;
	}
}