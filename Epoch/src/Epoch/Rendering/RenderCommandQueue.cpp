#include "epch.h"
#include "RenderCommandQueue.h"

namespace Epoch
{
	constexpr size_t BufferSize = 10 * 1024 * 1024; // 10mb buffer

	RenderCommandQueue::RenderCommandQueue()
	{
		myCommandBuffer = new uint8_t[BufferSize];
		myCommandBufferPtr = myCommandBuffer;
		memset(myCommandBuffer, 0, BufferSize);
	}

	RenderCommandQueue::~RenderCommandQueue()
	{
		delete[] myCommandBuffer;
	}

	bool Check(uint8_t* aPtr, uint8_t* aBegin, uint8_t* aEnd)
	{
		return std::less_equal<uint8_t*>{}(aBegin, aPtr) && std::less<uint8_t*>{}(aPtr, aEnd);
	}

	void* Epoch::RenderCommandQueue::Allocate(RenderCommandFn aFunc, uint32_t aSize)
	{
		const uint32_t cmdTotalSize = sizeof(RenderCommandFn) + sizeof(uint32_t) + aSize;

		EPOCH_ASSERT(Check(myCommandBufferPtr + cmdTotalSize, myCommandBuffer, myCommandBuffer + BufferSize), "RenderCommandQueue to small!");

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
