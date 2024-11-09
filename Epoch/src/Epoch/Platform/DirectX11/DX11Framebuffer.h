#pragma once
#include "Epoch/Rendering/Framebuffer.h"

namespace Epoch
{
	class DX11Framebuffer : public Framebuffer
	{
	public:
		DX11Framebuffer(const FramebufferSpecification& aSpec);
		~DX11Framebuffer() override = default;

		void Resize(uint32_t aWidth, uint32_t aHeight);
	};
}
