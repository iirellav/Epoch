#pragma once
#include "Epoch/Rendering/Texture.h"
#include <mutex>
#include <d3d11.h>
#include <wrl.h>

using namespace Microsoft::WRL;

namespace Epoch
{
	inline static DXGI_FORMAT DX11TextureFormat(TextureFormat aFormat)
	{
		switch (aFormat)
		{
		//case Epoch::TextureFormat::RGB:		return DXGI_FORMAT_R8G8B8_UNORM;
		case TextureFormat::RGBA:			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case TextureFormat::RGBA16F:		return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case TextureFormat::RGBA32F:		return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case TextureFormat::R11G11B10F:		return DXGI_FORMAT_R11G11B10_FLOAT;
		case TextureFormat::RGB10A2UNORM:	return DXGI_FORMAT_R10G10B10A2_UNORM;
		case TextureFormat::RG16F:			return DXGI_FORMAT_R16G16_FLOAT;
		case TextureFormat::R32F:			return DXGI_FORMAT_R32_FLOAT;
		case TextureFormat::R32UI:			return DXGI_FORMAT_R32_UINT;
		case TextureFormat::DEPTH32:		return DXGI_FORMAT_R32_TYPELESS;
		}
		EPOCH_ASSERT(false, "Unknown texture format!");
		return DXGI_FORMAT_UNKNOWN;
	}

	class DX11Texture2D : public Texture2D
	{
	public:
		DX11Texture2D() = default;
		DX11Texture2D(const std::filesystem::path& aFilepath);
		DX11Texture2D(const TextureSpecification& aSpec, Buffer aTextureData = Buffer());
		~DX11Texture2D() override;

		const void* const GetView() override { return mySRV.Get(); }
		ComPtr<ID3D11Resource> const GetTexture() { return myTexture; }
		ComPtr<ID3D11ShaderResourceView> const GetSRV() { return mySRV; }
		ComPtr<ID3D11RenderTargetView> const GetRTV() { return myRTV; }
		ComPtr<ID3D11UnorderedAccessView> const GetUAV() { return myUAV; }
		ComPtr<ID3D11DepthStencilView> const GetDSV() { return myDSV; }
		
		void Resize(uint32_t aWidth, uint32_t aHeight) override;
		void SetData(Buffer aTextureData) override;

		Buffer ReadData() const override;
		Buffer ReadData(uint32_t aWidth, uint32_t aHeight, uint32_t aX, uint32_t aY) const override;

	private:
		void Create();
		static std::shared_ptr<DX11Texture2D> CreateStagingTexture(TextureFormat aFormat, uint32_t aWidth, uint32_t aHeight);

	private:
		ComPtr<ID3D11Resource> myTexture = nullptr;
		ComPtr<ID3D11ShaderResourceView> mySRV = nullptr;
		ComPtr<ID3D11RenderTargetView> myRTV = nullptr;
		ComPtr<ID3D11UnorderedAccessView> myUAV = nullptr;
		ComPtr<ID3D11DepthStencilView> myDSV = nullptr;

		unsigned myBindFlags{};
		unsigned myUsageFlags{};
		unsigned myAccessFlags{};

		static inline std::mutex staticMutex;

		friend class RHI;
		friend class DX11Framebuffer;
		friend class DX11TextureCube;
	};

	class DX11TextureCube : public TextureCube
	{
	public:
		DX11TextureCube(const TextureSpecification& aSpec, Buffer aTextureData = Buffer());
		~DX11TextureCube() override;
		
		void GenerateMips();

		const void* const GetView() override { return mySRV.Get(); }

		ComPtr<ID3D11Resource> const GetTexture() { return myTexture; }
		ComPtr<ID3D11ShaderResourceView> const GetSRV() { return mySRV; }
		ComPtr<ID3D11RenderTargetView> const GetRTV() { return myRTV; }
		ComPtr<ID3D11UnorderedAccessView> const GetUAV() { return myUAV; }

		ComPtr<ID3D11UnorderedAccessView> const GetMipUAV(uint32_t aMipLevel);

		Buffer ReadData() const override;

	private:
		static std::shared_ptr<DX11Texture2D> CreateStagingTexture(TextureFormat aFormat, uint32_t aWidth, uint32_t aHeight);

	private:
		ComPtr<ID3D11Resource> myTexture = nullptr;
		ComPtr<ID3D11ShaderResourceView> mySRV = nullptr;
		ComPtr<ID3D11RenderTargetView> myRTV = nullptr;
		//std::vector<ComPtr<ID3D11RenderTargetView>> myRTVs;
		ComPtr<ID3D11UnorderedAccessView> myUAV = nullptr;

		unsigned myBindFlags{};
		unsigned myUsageFlags{};
		unsigned myAccessFlags{};

		static inline std::mutex staticMutex;
	};
}
