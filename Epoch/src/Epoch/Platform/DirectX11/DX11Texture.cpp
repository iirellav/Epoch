#include "epch.h"
#include "DX11Texture.h"
#include <d3dcommon.h>
#include "Epoch/Assets/TextureImporter.h"
#include "Epoch/Rendering/RHI.h" //TODO: Remove, this is disgusting

namespace Epoch
{
	DX11Texture2D::DX11Texture2D(const std::filesystem::path& aFilepath)
	{
		myTextureData = TextureImporter::ToBufferFromFile(aFilepath, mySpecification.format, mySpecification.width, mySpecification.height);
		mySpecification.debugName = aFilepath.stem().string();
		mySpecification.usage = TextureUsage::Texture;
		mySpecification.generateMips = true;

		if (!myTextureData)
		{
			LOG_WARNING("Failed to load texture data '{}'", aFilepath.string());
		}

		Create();
	}

	DX11Texture2D::DX11Texture2D(const TextureSpecification& aSpec, Buffer aTextureData)
	{
		mySpecification = aSpec;

		auto size = GetMemorySize(mySpecification.format, mySpecification.width, mySpecification.height);
		if (aTextureData)
		{
			myTextureData = Buffer::Copy(aTextureData.data, size);
		}

		Create();
	}

	DX11Texture2D::~DX11Texture2D()
	{
		if (myTextureData)
		{
			myTextureData.Release();
		}
	}

	void DX11Texture2D::Resize(uint32_t aWidth, uint32_t aHeight)
	{
		EPOCH_PROFILE_FUNC();

		EPOCH_ASSERT(aWidth > 0 && aHeight > 0, "Trying to resize a texture to an invalid size");

		if (mySpecification.width == aWidth && mySpecification.height == aHeight)
		{	
			return;
		}

		if (myTextureData)
		{
			myTextureData.Release();
		}

		if (mySRV)
		{
			mySRV.ReleaseAndGetAddressOf();
		}

		if (myRTV)
		{
			myRTV.ReleaseAndGetAddressOf();
		}

		if (myDSV)
		{
			myDSV.ReleaseAndGetAddressOf();
		}

		if (myTexture)
		{
			myTexture.ReleaseAndGetAddressOf();
		}

		mySpecification.width = aWidth;
		mySpecification.height = aHeight;

		Create();
	}

	void DX11Texture2D::SetData(Buffer aTextureData)
	{
		EPOCH_PROFILE_FUNC();

		auto size = GetMemorySize(mySpecification.format, mySpecification.width, mySpecification.height);

		if (aTextureData.size != size)
		{
			LOG_ERROR("Tried to set a textures data with invalid data size!");
			return;
		}

		myTextureData.Release();
		myTextureData = Buffer::Copy(aTextureData);

		if (mySRV)
		{
			mySRV.ReleaseAndGetAddressOf();
		}
		
		if (myRTV)
		{
			myRTV.ReleaseAndGetAddressOf();
		}
		
		if (myDSV)
		{
			myDSV.ReleaseAndGetAddressOf();
		}
		
		if (myTexture)
		{
			myTexture.ReleaseAndGetAddressOf();
		}
		
		Create();
	}

	Buffer DX11Texture2D::ReadData(uint32_t aWidth, uint32_t aHeight, uint32_t aX, uint32_t aY) const
	{
		Buffer outputData;

		auto stagingBuffer = CreateStagingTexture(mySpecification.format, aWidth, aHeight);

		if (!stagingBuffer)
		{
			return outputData;
		}
		
		auto dataSize = GetMemorySize(mySpecification.format, aWidth, aHeight);
		outputData.Allocate(dataSize);

		D3D11_BOX box = {};
		box.left = aX;
		box.right = aX + aWidth;
		box.top = aY;
		box.bottom = aY + aHeight;
		box.front = 0;
		box.back = 1;

		{
			ID3D11Resource* resource = nullptr;
			myRTV.Get()->GetResource(&resource);
			RHI::GetContext()->CopySubresourceRegion(stagingBuffer->myTexture.Get(), 0, 0, 0, 0, resource, 0, &box);
		}

		{
			D3D11_MAPPED_SUBRESOURCE resource;
			ZeroMemory(&resource, sizeof(D3D11_MAPPED_SUBRESOURCE));
			RHI::GetContext()->Map(stagingBuffer->myTexture.Get(), 0, D3D11_MAP_READ, 0, &resource);
			if (resource.pData)
			{
				memcpy(outputData.data, resource.pData, dataSize);
			}
			RHI::GetContext()->Unmap(stagingBuffer->myTexture.Get(), 0);
		}

		return outputData;
	}

	void DX11Texture2D::Create()
	{
		auto size = GetMemorySize(mySpecification.format, mySpecification.width, mySpecification.height);

		UINT bindFlags;
		if (mySpecification.usage == TextureUsage::Attachment)
		{
			if (IsDepthFormat(mySpecification.format))
			{
				bindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
			}
			else
			{
				bindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
			}
		}
		else if (mySpecification.usage == TextureUsage::Storage)
		{
			bindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_RENDER_TARGET;
		}
		else
		{
			bindFlags = D3D11_BIND_SHADER_RESOURCE;
		}

		D3D11_SUBRESOURCE_DATA subResourceData{};
		if (myTextureData)
		{
			subResourceData.pSysMem = myTextureData.data;
			subResourceData.SysMemPitch = (uint32_t)size / mySpecification.height;
			subResourceData.SysMemSlicePitch = 0;
		}

		DXGI_FORMAT format = DX11TextureFormat(mySpecification.format);
		UINT mipLevels = mySpecification.generateMips ? GetMipLevelCount() : 1;

		D3D11_TEXTURE2D_DESC description;
		ZeroMemory(&description, sizeof(description));
		description.Width = mySpecification.width;
		description.Height = mySpecification.height;
		description.MipLevels = mipLevels;
		description.ArraySize = 1;
		description.Format = format;
		description.SampleDesc.Count = 1;
		description.SampleDesc.Quality = 0;
		description.Usage = D3D11_USAGE_DEFAULT;
		description.BindFlags = mySpecification.generateMips ? bindFlags | D3D11_BIND_RENDER_TARGET : bindFlags;
		description.CPUAccessFlags = 0;
		description.MiscFlags = mySpecification.generateMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

		if (mySpecification.generateMips)
		{
			HRESULT result = RHI::GetDevice()->CreateTexture2D(&description, nullptr, reinterpret_cast<ID3D11Texture2D**>(myTexture.GetAddressOf()));
			if (FAILED(result))
			{
				EPOCH_ASSERT(false, "Failed to create a texture 2D!");
			}

			std::lock_guard lock(staticMutex);
			RHI::GetContext()->UpdateSubresource(myTexture.Get(), 0u, nullptr, myTextureData.data, (uint32_t)size / mySpecification.height, 0u);
		}
		else
		{
			HRESULT result = RHI::GetDevice()->CreateTexture2D(&description, myTextureData ? &subResourceData : nullptr, reinterpret_cast<ID3D11Texture2D**>(myTexture.GetAddressOf()));
			if (FAILED(result))
			{
				EPOCH_ASSERT(false, "Failed to create a texture 2D!");
			}
		}

		if (bindFlags & D3D11_BIND_DEPTH_STENCIL)
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC depthDescription = {};
			depthDescription.Format = DXGI_FORMAT_D32_FLOAT;
			depthDescription.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

			HRESULT result = RHI::GetDevice()->CreateDepthStencilView(myTexture.Get(), &depthDescription, myDSV.GetAddressOf());
			if (FAILED(result))
			{
				EPOCH_ASSERT(false, "Failed to create a depth stencil view!");
			}
		}

		if (bindFlags & D3D11_BIND_SHADER_RESOURCE)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC depthSRVDescription = {};
			depthSRVDescription.Format = DXGI_FORMAT_R32_FLOAT;
			depthSRVDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			depthSRVDescription.Texture2D.MipLevels = mipLevels;

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDescription;
			srvDescription.Format = description.Format;
			srvDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDescription.Texture2D.MipLevels = mipLevels;
			srvDescription.Texture2D.MostDetailedMip = 0;

			HRESULT result = RHI::GetDevice()->CreateShaderResourceView(myTexture.Get(), (bindFlags & D3D11_BIND_DEPTH_STENCIL) ? &depthSRVDescription : &srvDescription, mySRV.GetAddressOf());
			if (FAILED(result))
			{
				EPOCH_ASSERT(false, "Failed to create a shader resource view!");
			}
		}

		if (bindFlags & D3D11_BIND_RENDER_TARGET)
		{
			HRESULT result = RHI::GetDevice()->CreateRenderTargetView(myTexture.Get(), nullptr, myRTV.GetAddressOf());
			if (FAILED(result))
			{
				EPOCH_ASSERT(false, "Failed to create a render target view!");
			}
		}

		if (bindFlags & D3D11_BIND_UNORDERED_ACCESS)
		{
			HRESULT result = RHI::GetDevice()->CreateUnorderedAccessView(myTexture.Get(), nullptr, myUAV.GetAddressOf());
			if (FAILED(result))
			{
				EPOCH_ASSERT(false, "Failed to create a unordered access view!");
			}
		}

		if (mySpecification.generateMips && mipLevels > 1)
		{
			std::lock_guard lock(staticMutex);
			RHI::GetContext()->GenerateMips(mySRV.Get());
		}

		myBindFlags = bindFlags;
		myUsageFlags = D3D11_USAGE_DEFAULT;
		myAccessFlags = 0;

		D3D_SET_OBJECT_NAME_A(myTexture, mySpecification.debugName.data());
	}

	std::shared_ptr<DX11Texture2D> DX11Texture2D::CreateStagingTexture(TextureFormat aFormat, uint32_t aWidth, uint32_t aHeight)
	{
		std::shared_ptr<DX11Texture2D> output = std::make_shared<DX11Texture2D>();

		DXGI_FORMAT format = DX11TextureFormat(aFormat);

		D3D11_TEXTURE2D_DESC description;
		ZeroMemory(&description, sizeof(description));
		description.Width = aWidth;
		description.Height = aHeight;
		description.MipLevels = 1;
		description.ArraySize = 1;
		description.Format = format;
		description.SampleDesc.Count = 1;
		description.SampleDesc.Quality = 0;
		description.Usage = D3D11_USAGE_STAGING;
		description.BindFlags = 0;
		description.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		description.MiscFlags = 0;

		HRESULT result = RHI::GetDevice()->CreateTexture2D(&description, nullptr, reinterpret_cast<ID3D11Texture2D**>(output->myTexture.GetAddressOf()));
		if (FAILED(result))
		{
			EPOCH_ASSERT(false, "Failed to create a staging texture!");
			output = nullptr;
		}

		return output;
	}

	DX11TextureCube::DX11TextureCube(const TextureSpecification& aSpec, Buffer aTextureData)
	{
		mySpecification = aSpec;

		if (aTextureData)
		{
			myTextureData = Buffer::Copy(aTextureData.data, aTextureData.size);
		}

		D3D11_SUBRESOURCE_DATA subResourceData{};
		if (myTextureData)
		{
			subResourceData.pSysMem = myTextureData.data;
			subResourceData.SysMemPitch = (uint32_t)aSpec.width / 4;
			subResourceData.SysMemSlicePitch = 0;
		}

		UINT bindFlags;
		if (mySpecification.usage == TextureUsage::Attachment)
		{
			bindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		}
		if (mySpecification.usage == TextureUsage::Storage)
		{
			bindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_RENDER_TARGET;
		}
		else
		{
			bindFlags = D3D11_BIND_SHADER_RESOURCE;
		}

		//mySpecification.generateMips = mySpecification.generateMips && (bindFlags & D3D11_BIND_RENDER_TARGET) ? true : false;

		myFormat = DX11TextureFormat(mySpecification.format);
		UINT mipLevels = mySpecification.generateMips ? GetMipLevelCount() : 1;

		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = mySpecification.width;
		desc.Height = mySpecification.height;
		desc.MipLevels = mipLevels;
		desc.ArraySize = 6;
		desc.Format = myFormat;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = bindFlags;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS; // mySpecification.generateMips ? D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS : D3D11_RESOURCE_MISC_TEXTURECUBE;

		HRESULT result = RHI::GetDevice()->CreateTexture2D(&desc, nullptr, reinterpret_cast<ID3D11Texture2D**>(myTexture.GetAddressOf()));
		if (FAILED(result))
		{
			EPOCH_ASSERT(false, "Failed to create a texture 2D!");
		}

		if (bindFlags & D3D11_BIND_SHADER_RESOURCE)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			srvDesc.Format = desc.Format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MipLevels = mipLevels;
			srvDesc.TextureCube.MostDetailedMip = 0;

			result = RHI::GetDevice()->CreateShaderResourceView(myTexture.Get(), &srvDesc, mySRV.GetAddressOf());
			if (FAILED(result))
			{
				EPOCH_ASSERT(false, "Failed to create a shader resource view!");
			}
		}

		if (bindFlags & D3D11_BIND_RENDER_TARGET)
		{
			result = RHI::GetDevice()->CreateRenderTargetView(myTexture.Get(), nullptr, myRTV.GetAddressOf());
			if (FAILED(result))
			{
				EPOCH_ASSERT(false, "Failed to create a render target view!");
			}

			//for (uint32_t i = 0; i < mipLevels; i++)
			//{
			//	D3D11_RENDER_TARGET_VIEW_DESC rtvDescription;
			//	rtvDescription.Format = description.Format;
			//	rtvDescription.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			//	rtvDescription.Texture2DArray.MipSlice = i;
			//	rtvDescription.Texture2DArray.FirstArraySlice = 0;
			//	rtvDescription.Texture2DArray.ArraySize = 6;
			//
			//	auto rtv = myRTVs.emplace_back();
			//	result = RHI::GetDevice()->CreateRenderTargetView(myTexture.Get(), &rtvDescription, rtv.GetAddressOf());
			//	if (FAILED(result))
			//	{
			//		EPOCH_ASSERT(false, "Failed to create a render target view!");
			//	}
			//}
		}

		if (bindFlags & D3D11_BIND_UNORDERED_ACCESS)
		{
			result = RHI::GetDevice()->CreateUnorderedAccessView(myTexture.Get(), nullptr, myUAV.GetAddressOf());
			if (FAILED(result))
			{
				EPOCH_ASSERT(false, "Failed to create a unordered access view!");
			}
		}
		
		//RHI::GetContext()->GenerateMips(mySRV.Get());

		myBindFlags = bindFlags;
		myUsageFlags = D3D11_USAGE_DEFAULT;
		myAccessFlags = 0;

		D3D_SET_OBJECT_NAME_A(myTexture, mySpecification.debugName.data());
	}

	DX11TextureCube::~DX11TextureCube()
	{
	}

	void DX11TextureCube::GenerateMips()
	{
		const uint32_t mipCount = GetMipLevelCount();
		if (!mySpecification.generateMips || mipCount == 1)
		{
			LOG_ERROR("Could not generate mips!");
			return;
		}

		RHI::GetContext()->GenerateMips(mySRV.Get());
	}

	ComPtr<ID3D11UnorderedAccessView> const DX11TextureCube::GetMipUAV(uint32_t aMipLevel)
	{
		if (myBindFlags & D3D11_BIND_UNORDERED_ACCESS)
		{
			const uint32_t mipCount = GetMipLevelCount();
			D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
			desc.Format = myFormat;
			desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.MipSlice = CU::Math::Clamp(aMipLevel, 0u, mipCount);
			desc.Texture2DArray.FirstArraySlice = 0;
			desc.Texture2DArray.ArraySize = 6;

			ComPtr<ID3D11UnorderedAccessView> output = nullptr;

			HRESULT result = RHI::GetDevice()->CreateUnorderedAccessView(myTexture.Get(), &desc, output.GetAddressOf());
			if (FAILED(result))
			{
				EPOCH_ASSERT(false, "Failed to create a unordered access view!");
			}

			return output;
		}

		return nullptr;
	}
}
