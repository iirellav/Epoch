#pragma once
#include <memory>
#include <CommonUtilities/Color.h>
#include "Epoch/Rendering/Framebuffer.h"

namespace Epoch
{
	class Window;

	class GraphicsEngine
	{
	public:
		bool Init(const CU::Color& aClearColor = CU::Color::Black);

		static GraphicsEngine& Get() { static GraphicsEngine instance; return instance; }

		std::shared_ptr<Texture2D> GetBackBuffer() const { return myBackBuffer; }
		std::shared_ptr<Texture2D> GetDepthBuffer() const { return myDepthBuffer; }

		void SetClearColor(const CU::Color& aColor) { myClearColor = aColor; }
		bool& GetVSyncBool() { return myVSync; }

		void BeginFrame();
		void EndFrame();

		bool OnWindowResize();

	private:
		GraphicsEngine() = default;
		~GraphicsEngine() = default;

	private:
		bool myIsInitialized = false;
		CU::Color myClearColor;
		bool myVSync = false;

		std::shared_ptr<Texture2D> myBackBuffer; 
		std::shared_ptr<Texture2D> myDepthBuffer;
	};
}
