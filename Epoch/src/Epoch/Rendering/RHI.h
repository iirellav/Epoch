#pragma once
#include <string>
#include <dxgi.h>
#include <d3d11.h>
#include <wrl.h>
#include "Epoch/Platform/DirectX11/DX11Texture.h"

using namespace Microsoft::WRL;

typedef
enum PIPELINE_STAGE
{
	PIPELINE_STAGE_VERTEX_SHADER = 1 << 1,
	PIPELINE_STAGE_PIXEL_SHADER = 1 << 2,
	PIPELINE_STAGE_GEOMETRY_SHADER = 1 << 3,
	PIPELINE_STAGE_COMPUTE_SHADER = 1 << 3,
} 	PIPELINE_STAGE;

namespace Epoch
{
	class Window;
	class Texture2D;

	class RHI
	{
	public:
		RHI() = delete;
		~RHI() = delete;

		static bool Init(DX11Texture2D* outBackBuffer, DX11Texture2D* outDepthBuffer, bool aEnableDeviceDebug = false);

		static bool CreateTexture(DX11Texture2D* outTexture, const std::string& aName, UINT aWidth, UINT aHeight, UINT aFormat,
			D3D11_USAGE aUsageFlags = D3D11_USAGE_DEFAULT, UINT aBindFlags = D3D11_BIND_SHADER_RESOURCE, UINT aCpuAccessFlags = 0);

		static void SetRenderTarget(const DX11Texture2D* aTarget, const DX11Texture2D* aDepthStencil);

		static void ClearRenderTarget(const DX11Texture2D* aTexture, std::array<float, 4> aClearColor = { 0.0f, 0.0f, 0.0f, 0.0f });
		static void ClearDepthStencil(const DX11Texture2D* aTexture);

		static bool ResizeDevice(DX11Texture2D* outBackBuffer, DX11Texture2D* outDepthBuffer);

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
