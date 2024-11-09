#include "epch.h"
#include "RHI.h"
#include <guiddef.h>
#include <d3d11shader.h>
#include <d3dcompiler.h>
#include <d3d11_1.h>
#include "Epoch/Core/Window.h"
#include "Epoch/Core/Application.h"

namespace Epoch
{
	constexpr char BackBufferDefaultName[] = "BackBuffer";
	constexpr char DepthBufferDefaultName[] = "DepthBuffer";

	bool RHI::Init(DX11Texture2D* outBackBuffer, DX11Texture2D* outDepthBuffer, bool aEnableDeviceDebug)
	{
		if (myIsInitialized)
		{
			LOG_ERROR("Called RHI::Init more than once!");
			return false;
		}

		EPOCH_PROFILE_FUNC();
		EPOCH_ASSERT(outBackBuffer && outDepthBuffer, "Please initialize the textures before calling this function!");

		HRESULT result = E_FAIL;

		// Swap chain and Device
		{
			DXGI_SWAP_CHAIN_DESC swapChainDescription = {};
			swapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDescription.OutputWindow = Application::Get().GetWindow().GetWin32Window();
			swapChainDescription.SampleDesc.Count = 1;
			swapChainDescription.Windowed = true;
			swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			swapChainDescription.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
			swapChainDescription.BufferCount = 2;

			result = D3D11CreateDeviceAndSwapChain
			(
				nullptr,
				D3D_DRIVER_TYPE_HARDWARE,
				nullptr,
				aEnableDeviceDebug ? D3D11_CREATE_DEVICE_DEBUG : 0,
				nullptr,
				0,
				D3D11_SDK_VERSION,
				&swapChainDescription,
				&mySwapChain,
				&myDevice,
				nullptr,
				&myContext
			);

			if (FAILED(result))
			{
				EPOCH_ASSERT(false, "Failed to create device and swap chain!");
				return false;
			}
		}

		unsigned width = 0;
		unsigned height = 0;

		// Back buffer
		{
			result = mySwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &outBackBuffer->myTexture);
			if (FAILED(result))
			{
				EPOCH_ASSERT(false, "Failed to retrieve backbuffer resource!");
				return false;
			}

			result = myDevice->CreateRenderTargetView(outBackBuffer->myTexture.Get(), nullptr, outBackBuffer->myRTV.GetAddressOf());
			if (FAILED(result))
			{
				EPOCH_ASSERT(false, "Failed to create RTV for backbuffer!");
				return false;
			}

			D3D_SET_OBJECT_NAME_A(outBackBuffer->myTexture, BackBufferDefaultName);

			D3D11_TEXTURE2D_DESC description = {};
			ID3D11Texture2D* tempPointer = nullptr;
			outBackBuffer->myTexture->QueryInterface<ID3D11Texture2D>(&tempPointer);
			tempPointer->GetDesc(&description);
			tempPointer->Release();

			width = description.Width;
			height = description.Height;

			outBackBuffer->mySpecification.debugName = BackBufferDefaultName;
			outBackBuffer->mySpecification.width = width;
			outBackBuffer->mySpecification.height = height;
			outBackBuffer->myBindFlags = D3D11_BIND_RENDER_TARGET;
			outBackBuffer->myUsageFlags = D3D11_USAGE_DEFAULT;
			outBackBuffer->myAccessFlags = 0;
			outBackBuffer->myViewport = D3D11_VIEWPORT({ 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f });
		}

		// Depth buffer
		{
			if (!CreateTexture(outDepthBuffer, DepthBufferDefaultName, width, height,
				DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL))
			{
				EPOCH_ASSERT(false, "Failed to create depth buffer!");
				return false;
			}

			outDepthBuffer->mySpecification.debugName = DepthBufferDefaultName;
			outDepthBuffer->mySpecification.width = width;
			outDepthBuffer->mySpecification.height = height;
			outDepthBuffer->myBindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
			outDepthBuffer->myUsageFlags = D3D11_USAGE_DEFAULT;
			outDepthBuffer->myAccessFlags = 0;
			outDepthBuffer->myViewport = D3D11_VIEWPORT({ 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f });
		}

		myIsInitialized = true;
		return true;
	}

	bool RHI::CreateTexture(DX11Texture2D* outTexture, const std::string& aName, UINT aWidth, UINT aHeight, UINT aFormat, D3D11_USAGE aUsageFlags, UINT aBindFlags, UINT aCpuAccessFlags)
	{
		EPOCH_ASSERT(outTexture, "Initialize the texture before calling this function!");

		EPOCH_PROFILE_FUNC();

		outTexture->mySpecification.debugName = aName;
		outTexture->mySpecification.width = aWidth;
		outTexture->mySpecification.height = aHeight;
		outTexture->myBindFlags = aBindFlags;
		outTexture->myUsageFlags = aUsageFlags;
		outTexture->myAccessFlags = aCpuAccessFlags;

		HRESULT result = S_FALSE;

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = aWidth;
		desc.Height = aHeight;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = static_cast<DXGI_FORMAT>(aFormat);
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = aUsageFlags;
		desc.BindFlags = aBindFlags;
		desc.CPUAccessFlags = aCpuAccessFlags;
		desc.MiscFlags = 0;

		result = myDevice->CreateTexture2D(&desc, nullptr, reinterpret_cast<ID3D11Texture2D**>(outTexture->myTexture.GetAddressOf()));
		if (FAILED(result))
		{
			EPOCH_ASSERT(false, "Failed to create the requested texture!");
			return false;
		}

		if (aBindFlags & D3D11_BIND_DEPTH_STENCIL)
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC depthDesc = {};
			depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
			depthDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

			result = myDevice->CreateDepthStencilView(outTexture->myTexture.Get(), &depthDesc, outTexture->myDSV.GetAddressOf());
			if (FAILED(result))
			{
				EPOCH_ASSERT(false, "Failed to create a depth stencil view!");
				return false;
			}
		}

		if (aBindFlags & D3D11_BIND_SHADER_RESOURCE)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC depthSRVDesc = {};
			depthSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
			depthSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			depthSRVDesc.Texture2D.MipLevels = desc.MipLevels;

			result = myDevice->CreateShaderResourceView(outTexture->myTexture.Get(), (aBindFlags & D3D11_BIND_DEPTH_STENCIL) ? &depthSRVDesc : nullptr, outTexture->mySRV.GetAddressOf());
			if (FAILED(result))
			{
				EPOCH_ASSERT(false, "Failed to create a shader resource view!");
				return false;
			}
		}

		if (aBindFlags & D3D11_BIND_RENDER_TARGET)
		{
			result = myDevice->CreateRenderTargetView(outTexture->myTexture.Get(), nullptr, outTexture->myRTV.GetAddressOf());
			if (FAILED(result))
			{
				EPOCH_ASSERT(false, "Failed to create a render target view!");
				return false;
			}
		}

		outTexture->myViewport = D3D11_VIEWPORT({ 0.0f, 0.0f, static_cast<float>(aWidth), static_cast<float>(aHeight), 0.0f, 1.0f });

		D3D_SET_OBJECT_NAME_A(outTexture->myTexture, outTexture->mySpecification.debugName.data());

		return true;
	}

	void RHI::SetRenderTarget(const DX11Texture2D* aTarget, const DX11Texture2D* aDepthStencil)
	{
		myContext->OMSetRenderTargets(aTarget ? 1 : 0, aTarget ? aTarget->myRTV.GetAddressOf() : nullptr, aDepthStencil ? aDepthStencil->myDSV.Get() : nullptr);

		if (aTarget)
		{
			myContext->RSSetViewports(1, &aTarget->myViewport);
		}
		else if (aDepthStencil)
		{
			myContext->RSSetViewports(1, &aDepthStencil->myViewport);
		}
	}

	void RHI::ClearRenderTarget(const DX11Texture2D* aTexture, std::array<float, 4> aClearColor)
	{
		if ((aTexture->myBindFlags & D3D11_BIND_RENDER_TARGET) == false)
		{
			EPOCH_ASSERT(false, "Attempted to clear a read-only texture!");
			return;
		}

		myContext->ClearRenderTargetView(aTexture->myRTV.Get(), aClearColor.data());
	}

	void RHI::ClearDepthStencil(const DX11Texture2D* aTexture)
	{
		if ((aTexture->myBindFlags & D3D11_BIND_DEPTH_STENCIL) == false)
		{
			EPOCH_ASSERT(false, "Attempted to clear depth on a non-depth texture!");
			return;
		}

		myContext->ClearDepthStencilView(aTexture->myDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}

	bool RHI::ResizeDevice(DX11Texture2D* outBackBuffer, DX11Texture2D* outDepthBuffer)
	{
		EPOCH_ASSERT(outBackBuffer && outDepthBuffer, "Please initialize the textures before calling this function!");
		EPOCH_PROFILE_FUNC();

		myContext->OMSetRenderTargets(0, 0, 0);

		HRESULT result = E_FAIL;

		result = mySwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
		if (FAILED(result))
		{
			EPOCH_ASSERT(false, "Failed to resize buffers!");
			return false;
		}

		unsigned width = 0;
		unsigned height = 0;

		// Back buffer
		{
			result = mySwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &outBackBuffer->myTexture);
			if (FAILED(result))
			{
				EPOCH_ASSERT(false, "Failed to retrieve backbuffer resource!");
				return false;
			}

			result = myDevice->CreateRenderTargetView(outBackBuffer->myTexture.Get(), nullptr, outBackBuffer->myRTV.GetAddressOf());
			if (FAILED(result))
			{
				EPOCH_ASSERT(false, "Failed to create RTV for backbuffer!");
				return false;
			}

			D3D_SET_OBJECT_NAME_A(outBackBuffer->myTexture, BackBufferDefaultName);

			D3D11_TEXTURE2D_DESC description = {};
			ID3D11Texture2D* tempPointer = nullptr;
			outBackBuffer->myTexture->QueryInterface<ID3D11Texture2D>(&tempPointer);
			tempPointer->GetDesc(&description);
			tempPointer->Release();

			height = description.Height;
			width = description.Width;

			outBackBuffer->mySpecification.debugName = BackBufferDefaultName;
			outBackBuffer->mySpecification.width = width;
			outBackBuffer->mySpecification.height = height;
			outBackBuffer->myBindFlags = D3D11_BIND_RENDER_TARGET;
			outBackBuffer->myUsageFlags = D3D11_USAGE_DEFAULT;
			outBackBuffer->myAccessFlags = 0;
			outBackBuffer->myViewport = D3D11_VIEWPORT({ 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f });
		}

		// Depth buffer
		{
			if (!CreateTexture(outDepthBuffer, DepthBufferDefaultName, width, height,
				DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL))
			{
				EPOCH_ASSERT(false, "Failed to create depth buffer!");
				return false;
			}

			outDepthBuffer->mySpecification.debugName = DepthBufferDefaultName;
			outDepthBuffer->mySpecification.width = width;
			outDepthBuffer->mySpecification.height = height;
			outDepthBuffer->myBindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
			outDepthBuffer->myUsageFlags = D3D11_USAGE_DEFAULT;
			outDepthBuffer->myAccessFlags = 0;
			outDepthBuffer->myViewport = D3D11_VIEWPORT({ 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f });
		}

		return true;
	}

	void RHI::Present(bool aVSync)
	{
		if (aVSync)
		{
			mySwapChain->Present(1, 0);
		}
		else
		{
			mySwapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
		}
	}
}
