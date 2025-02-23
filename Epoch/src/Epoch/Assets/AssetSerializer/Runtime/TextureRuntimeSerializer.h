#pragma once
#include "Epoch/Serialization/FileStream.h"

namespace Epoch
{
	class Texture2D;
	class TextureCube;

	class TextureRuntimeSerializer
	{
	public:
		struct TextureMetadata
		{
			uint32_t width;
			uint32_t height;
			uint16_t format;
		};

	public:
		static uint64_t SerializeTexture2DToFile(std::shared_ptr<Texture2D> aTexture, FileStreamWriter& aStream);
		static std::shared_ptr<Texture2D> DeserializeTexture2D(FileStreamReader& aStream);

		static uint64_t SerializeTextureCubeToFile(std::shared_ptr<TextureCube> aTexture, FileStreamWriter& aStream);
		static std::shared_ptr<TextureCube> DeserializeTextureCube(FileStreamReader& aStream);
	};
}
