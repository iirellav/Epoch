#include "epch.h"
#include "Texture.h"
#include "Epoch/Rendering/RendererAPI.h"
#include "Epoch/Platform/DirectX11/DX11Texture.h"

namespace Epoch
{
	std::shared_ptr<Texture2D> Texture2D::Create(const std::filesystem::path& aFilepath)
	{
		EPOCH_PROFILE_FUNC();

		switch (RendererAPI::Current())
		{
		case RendererAPIType::DirectX11: return std::make_shared<DX11Texture2D>(aFilepath);
		}
		EPOCH_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	std::shared_ptr<Texture2D> Texture2D::Create(const TextureSpecification& aSpec, Buffer aTextureData)
	{
		EPOCH_PROFILE_FUNC();

		switch (RendererAPI::Current())
		{
		case RendererAPIType::DirectX11: return std::make_shared<DX11Texture2D>(aSpec, aTextureData);
		}
		EPOCH_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	std::pair<uint32_t, uint32_t> Texture2D::GetMipSize(uint32_t aMipLevel) const
	{
		uint32_t width = mySpecification.width;
		uint32_t height = mySpecification.height;

		while (aMipLevel != 0)
		{
			width /= 2;
			height /= 2;
			aMipLevel--;
		}

		return { width, height };
	}

	std::shared_ptr<TextureCube> TextureCube::Create(const TextureSpecification& aSpec, Buffer aTextureData)
	{
		EPOCH_PROFILE_FUNC();

		switch (RendererAPI::Current())
		{
		case RendererAPIType::DirectX11: return std::make_shared<DX11TextureCube>(aSpec, aTextureData);
		}
		EPOCH_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	std::pair<uint32_t, uint32_t> TextureCube::GetMipSize(uint32_t aMipLevel) const
	{
		uint32_t width = mySpecification.width;
		uint32_t height = mySpecification.height;

		while (aMipLevel != 0)
		{
			width /= 2;
			height /= 2;
			aMipLevel--;
		}

		return { width, height };
	}
}
