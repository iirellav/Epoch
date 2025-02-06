#include "epch.h"
#include "RenderCommandQueue.h"

namespace Epoch
{
	RenderCommandQueue::RenderCommandQueue()
	{
		size_t bufferSize = 10 * 1024 * 1024; // 10mb buffer

		myCommandBuffer = new uint8_t[bufferSize];
		myCommandBufferPtr = myCommandBuffer;
		memset(myCommandBuffer, 0, bufferSize);
	}

	RenderCommandQueue::~RenderCommandQueue()
	{
		delete[] myCommandBuffer;
	}

	void* Epoch::RenderCommandQueue::Allocate(RenderCommandFn aFunc, uint32_t aSize)
	{
		*(RenderCommandFn*)myCommandBufferPtr = aFunc;
		myCommandBufferPtr += sizeof(RenderCommandFn);

		*(uint32_t*)myCommandBufferPtr = aSize;
		myCommandBufferPtr += sizeof(uint32_t);

		void* memory = myCommandBufferPtr;
		myCommandBufferPtr += aSize;

		myCommandCount++;
		return memory;
	}
	
	void Epoch::RenderCommandQueue::Execute()
	{
		byte* buffer = myCommandBuffer;

		for (uint32_t i = 0; i < myCommandCount; i++)
		{
			RenderCommandFn function = *(RenderCommandFn*)buffer;
			buffer += sizeof(RenderCommandFn);

			uint32_t size = *(uint32_t*)buffer;
			buffer += sizeof(uint32_t);
			function(buffer);
			buffer += size;
		}

		myCommandBufferPtr = myCommandBuffer;
		myCommandCount = 0;
	}
}
