#include "epch.h"
#include "VertexBuffer.h"
#include "Epoch/Rendering/RendererAPI.h"
#include "Epoch/Platform/DirectX11/DX11VertexBuffer.h"

namespace Epoch
{
	std::shared_ptr<VertexBuffer> Epoch::VertexBuffer::Create(void* aData, uint32_t aCount, uint32_t aStride, VertexBufferUsage aUsage)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::DirectX11: return std::make_shared<DX11VertexBuffer>(aData, aCount, aStride, aUsage);
		}
		EPOCH_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	std::shared_ptr<VertexBuffer> Epoch::VertexBuffer::Create(uint32_t aCount, uint32_t aStride)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::DirectX11: return std::make_shared<DX11VertexBuffer>(aCount, aStride);
		}
		EPOCH_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}
