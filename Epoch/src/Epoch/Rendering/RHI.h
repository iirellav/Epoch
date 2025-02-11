#pragma once
#include <string>
#include <dxgi.h>
#include <d3d11.h>
#include <wrl.h>
#include "Epoch/Platform/DirectX11/DX11Texture.h"

using namespace Microsoft::WRL;

namespace Epoch
{
	class Window;
	class Texture2D;

	class RHI
	{
	public:
		RHI() = delete;
		~RHI() = delete;

		static bool Init(DX11Texture2D* outBackBuffer, bool aEnableDeviceDebug = false);

		static void SetAsRenderTarget(const DX11Texture2D* aTarget);

		static void Clear(const DX11Texture2D* aTexture, std::array<float, 4> aClearColor = { 0.0f, 0.0f, 0.0f, 0.0f });

		static bool Resize(DX11Texture2D* outBackBuffer);

		static void Present(bool aVSync = true);

		static ComPtr<ID3D11Device>& GetDevice() { return myDevice; }
		static ComPtr<ID3D11DeviceContext>& GetContext() { return myContext; }

	private:
		inline static bool myIsInitialized = false;

		inline static ComPtr<ID3D11Device> myDevice;
		inline static ComPtr<ID3D11DeviceContext> myContext;
		inline static ComPtr<IDXGISwapChain> mySwapChain;
	};
}
