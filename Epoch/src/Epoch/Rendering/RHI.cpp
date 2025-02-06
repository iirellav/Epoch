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

	bool RHI::Init(DX11Texture2D* outBackBuffer, bool aEnableDeviceDebug)
	{
		if (myIsInitialized)
		{
			LOG_ERROR("Called RHI::Init more than once!");
			return false;
		}

		EPOCH_PROFILE_FUNC();
		EPOCH_ASSERT(outBackBuffer, "Please initialize the textures before calling this function!");

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

			outBackBuffer->mySpecification.debugName = BackBufferDefaultName;
			outBackBuffer->mySpecification.width = description.Width;
			outBackBuffer->mySpecification.height = description.Height;
			outBackBuffer->myBindFlags = D3D11_BIND_RENDER_TARGET;
			outBackBuffer->myUsageFlags = D3D11_USAGE_DEFAULT;
			outBackBuffer->myAccessFlags = 0;
		}

		myIsInitialized = true;
		return true;
	}

	void RHI::SetAsRenderTarget(const DX11Texture2D* aTarget)
	{
		myContext->OMSetRenderTargets(1, aTarget->myRTV.GetAddressOf(), nullptr);

		const D3D11_VIEWPORT viewport(0.0f, 0.0f, (float)aTarget->GetWidth(), (float)aTarget->GetHeight(), 0.0f, 1.0f);
		myContext->RSSetViewports(1, &viewport);
	}

	void RHI::Clear(const DX11Texture2D* aTexture, std::array<float, 4> aClearColor)
	{
		if ((aTexture->myBindFlags & D3D11_BIND_RENDER_TARGET) == false)
		{
			EPOCH_ASSERT(false, "Attempted to clear a read-only texture!");
			return;
		}

		myContext->ClearRenderTargetView(aTexture->myRTV.Get(), aClearColor.data());
	}

	bool RHI::Resize(DX11Texture2D* outBackBuffer)
	{
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
