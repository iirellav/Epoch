#include "epch.h"
#include "IndexBuffer.h"
#include "Epoch/Rendering/RendererAPI.h"
#include "Epoch/Platform/DirectX11/DX11IndexBuffer.h"

namespace Epoch
{
	std::shared_ptr<IndexBuffer> IndexBuffer::Create(void* aData, uint32_t aCount, IndexBufferUsage aUsage)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::DirectX11: return std::make_shared<DX11IndexBuffer>(aData, aCount);
		}
		EPOCH_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	std::shared_ptr<IndexBuffer> IndexBuffer::Create(uint32_t aCount)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::DirectX11: return std::make_shared<DX11IndexBuffer>(aCount);
		}
		EPOCH_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}
