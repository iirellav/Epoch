#pragma once
#include <cstdint>

namespace Epoch
{
	typedef void(*RenderCommandFn)(void*);

	class RenderCommandQueue
	{
	public:
		RenderCommandQueue();
		~RenderCommandQueue();

		void* Allocate(RenderCommandFn aFunc, uint32_t aSize);

		void Execute();

	private:
		uint8_t* myCommandBuffer;
		uint8_t* myCommandBufferPtr;
		uint32_t myCommandCount = 0;
	};
}
