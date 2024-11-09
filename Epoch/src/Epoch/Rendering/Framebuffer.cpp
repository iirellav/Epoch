#include "epch.h"
#include "Framebuffer.h"
#include "Epoch/Rendering/RendererAPI.h"
#include "Epoch/Platform/DirectX11/DX11Framebuffer.h"

namespace Epoch
{
	std::shared_ptr<Framebuffer> Framebuffer::Create(const FramebufferSpecification& aSpec)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::DirectX11: return std::make_shared<DX11Framebuffer>(aSpec);
		}
		EPOCH_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}
