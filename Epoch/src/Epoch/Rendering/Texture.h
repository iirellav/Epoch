#pragma once
#include <string>
#include <filesystem>
#include <CommonUtilities/Math/CommonMath.hpp>
#include "Epoch/Assets/Asset.h"
#include "Epoch/Core/Buffer.h"

namespace Epoch
{
	enum class TextureFormat
	{
		None = 0,
		//RGB,
		RGBA,
		RGBA16F,
		RGBA32F,
		R11G11B10F,
		RGB10A2UNORM,
		RG16F,
		R32F,
		R32UI,
		
		DEPTH32
	};

	enum class TextureUsage
	{
		None = 0,
		Texture,
		Attachment,
		Storage
	};

	static bool IsDepthFormat(TextureFormat aFormat)
	{
		if (aFormat == TextureFormat::DEPTH32)
		{
			return true;
		}
		return false;
	}

	inline static size_t GetMemorySize(TextureFormat format, uint32_t width, uint32_t height)
	{
		switch (format)
		{
			//case TextureFormat::RGB:			return width * height * 3;
		case TextureFormat::RGBA:			return width * height * 4;
		case TextureFormat::RGBA16F:		return width * height * 4 * sizeof(uint16_t);
		case TextureFormat::RGBA32F:		return width * height * 4 * sizeof(uint32_t);
		case TextureFormat::R11G11B10F:		return width * height * 4;
		case TextureFormat::RGB10A2UNORM:	return width * height * 4;
		case TextureFormat::RG16F:			return width * height * 2;
		case TextureFormat::R32F:			return width * height * sizeof(uint32_t);
		case TextureFormat::R32UI:			return width * height * sizeof(uint32_t);
		case TextureFormat::DEPTH32:		return width * height * sizeof(uint32_t);
		}
		EPOCH_ASSERT(false, "Unknown texture format!");
		return 0;
	}
	
	struct TextureSpecification
	{
		TextureFormat format = TextureFormat::RGBA;
		TextureUsage usage = TextureUsage::Texture;
		uint32_t width = 1;
		uint32_t height = 1;
		
		bool generateMips = false;

		std::string debugName;
	};

	class Texture : public Asset
	{
	public:
		Texture() = default;
		virtual ~Texture() = default;
		
		virtual TextureFormat GetFormat() const = 0;
		virtual const void* const GetView() = 0;
	};

	class Texture2D : public Texture
	{
	public:
		static std::shared_ptr<Texture2D> Create(const std::filesystem::path& aFilepath);
		static std::shared_ptr<Texture2D> Create(const TextureSpecification& aSpec, Buffer aTextureData = Buffer());
		
		uint32_t GetWidth() const { return mySpecification.width; }
		uint32_t GetHeight() const { return mySpecification.height; }
		
		TextureFormat GetFormat() const override { return mySpecification.format; }
		bool Loaded() const { return myTextureData; }

		const Buffer GetReadableBuffer() { return myTextureData; }
		Buffer GetWriteableBuffer() { return myTextureData; }
		virtual void Resize(uint32_t aWidth, uint32_t aHeight) = 0;
		virtual void SetData(Buffer aTextureData) = 0;

		virtual Buffer ReadData(uint32_t aWidth, uint32_t aHeight, uint32_t aX, uint32_t aY) const = 0;

		uint32_t GetMipLevelCount() const { return CU::Math::FloorToUInt(log2f((float)CU::Math::Min(mySpecification.width, mySpecification.height)) + 1); }
		std::pair<uint32_t, uint32_t> GetMipSize(uint32_t aMipLevel) const;

		static AssetType GetStaticType() { return AssetType::Texture; }
		AssetType GetAssetType() const override { return GetStaticType(); }

	protected:
		TextureSpecification mySpecification;
		Buffer myTextureData;
	};

	class TextureCube : public Texture
	{
	public:
		static std::shared_ptr<TextureCube> Create(const TextureSpecification& aSpec, Buffer aTextureData = Buffer());
		
		unsigned GetWidth() const { return mySpecification.width; }
		unsigned GetHeight() const { return mySpecification.height; }
		
		TextureFormat GetFormat() const override { return mySpecification.format; }
		bool Loaded() const { return myTextureData; }
	
		uint32_t GetMipLevelCount() const { return CU::Math::FloorToUInt(log2f((float)CU::Math::Min(mySpecification.width, mySpecification.height)) + 1); }
		std::pair<uint32_t, uint32_t> GetMipSize(uint32_t aMipLevel) const;

		static AssetType GetStaticType() { return AssetType::EnvTexture; }
		AssetType GetAssetType() const override { return GetStaticType(); }
	
	protected:
		TextureSpecification mySpecification;
		Buffer myTextureData;
	};
}
