#include "epch.h"
#include "GraphicsEngine.h"
#include "Epoch/Rendering/RHI.h"

namespace Epoch
{
#ifndef _DEBUG
	constexpr bool dxDebug = false;
#else
	constexpr bool dxDebug = true;
#endif

	bool GraphicsEngine::Init(const CU::Color& aClearColor)
	{
		if (myIsInitialized)
		{
			LOG_ERROR("Called GraphicsEngine::Init more than once!");
			return false;
		}

		EPOCH_PROFILE_FUNC();

		myClearColor = aClearColor;

		myBackBuffer = std::make_shared<DX11Texture2D>();

		if (!RHI::Init((DX11Texture2D*)myBackBuffer.get(), dxDebug))
		{
			EPOCH_ASSERT(false, "Failed to initialize RHI!");
			return false;
		}

		myIsInitialized = true;
		return true;
	}

	void GraphicsEngine::BeginFrame()
	{
		EPOCH_PROFILE_FUNC();
		
		RHI::Clear((DX11Texture2D*)myBackBuffer.get(), { myClearColor.r, myClearColor.g, myClearColor.b, 1.0f });
	}

	void GraphicsEngine::EndFrame()
	{
		EPOCH_PROFILE_SCOPE("Present");

		RHI::Present(myVSync);
	}

	bool GraphicsEngine::OnWindowResize()
	{
		EPOCH_PROFILE_FUNC();

		EPOCH_ASSERT(myBackBuffer.use_count() == 1);

		myBackBuffer.reset();
		myBackBuffer = std::make_shared<DX11Texture2D>();

		return RHI::Resize((DX11Texture2D*)myBackBuffer.get());
	}
}
