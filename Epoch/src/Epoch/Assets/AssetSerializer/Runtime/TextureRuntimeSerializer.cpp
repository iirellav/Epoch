#include "epch.h"
#include "TextureRuntimeSerializer.h"
#include "Epoch/Rendering/Texture.h"

namespace Epoch
{
    uint64_t TextureRuntimeSerializer::SerializeTexture2DToFile(std::shared_ptr<Texture2D> aTexture, FileStreamWriter& aStream)
    {
		TextureMetadata metadata;
		metadata.width = aTexture->GetWidth();
		metadata.height = aTexture->GetHeight();
		metadata.format = (uint16_t)aTexture->GetFormat();

		Buffer imageBuffer;

		bool owningBuffer = false;
		if (aTexture->Loaded())
		{
			imageBuffer = aTexture->GetReadableBuffer();
		}
		else
		{
			owningBuffer = true;
			imageBuffer = aTexture->ReadData(aTexture->GetWidth(), aTexture->GetHeight(), 0, 0);
		}

		uint64_t startPosition = aStream.GetStreamPosition();
		aStream.WriteRaw(metadata);
		aStream.WriteBuffer(imageBuffer);
		uint64_t writtenSize = aStream.GetStreamPosition() - startPosition;

		if (owningBuffer)
		{
			imageBuffer.Release();
		}
		return writtenSize;
    }

    std::shared_ptr<Texture2D> TextureRuntimeSerializer::DeserializeTexture2D(FileStreamReader& aStream)
    {
		TextureMetadata metadata;
		aStream.ReadRaw<TextureMetadata>(metadata);

		Buffer buffer;
		aStream.ReadBuffer(buffer);

		TextureSpecification spec;
		spec.width = metadata.width;
		spec.height = metadata.height;
		spec.format = (TextureFormat)metadata.format;
		spec.generateMips = true;

		std::shared_ptr<Texture2D> texture = Texture2D::Create(spec, buffer);
		buffer.Release();
		return texture;
    }

	uint64_t TextureRuntimeSerializer::SerializeTextureCubeToFile(std::shared_ptr<TextureCube> aTexture, FileStreamWriter& aStream)
	{
		TextureMetadata metadata;
		metadata.width = aTexture->GetWidth();
		metadata.height = aTexture->GetHeight();
		metadata.format = (uint16_t)aTexture->GetFormat();

		Buffer imageBuffer;
		imageBuffer = aTexture->ReadData();

		uint64_t startPosition = aStream.GetStreamPosition();
		aStream.WriteRaw(metadata);
		aStream.WriteBuffer(imageBuffer);
		uint64_t writtenSize = aStream.GetStreamPosition() - startPosition;

		imageBuffer.Release();
		return writtenSize;

		return 0;
	}

	std::shared_ptr<TextureCube> TextureRuntimeSerializer::DeserializeTextureCube(FileStreamReader& aStream)
	{
		TextureMetadata metadata;
		aStream.ReadRaw<TextureMetadata>(metadata);
		Buffer buffer;
		aStream.ReadBuffer(buffer);

		TextureSpecification spec;
		spec.width = metadata.width;
		spec.height = metadata.height;
		spec.format = (TextureFormat)metadata.format;
		spec.generateMips = true;

		std::shared_ptr<TextureCube> texture = TextureCube::Create(spec, buffer);
		buffer.Release();
		return texture;
	}
}