#include "epch.h"
#include "ConstantBuffer.h"
#include "Epoch/Rendering/RendererAPI.h"
#include "Epoch/Platform/DirectX11/DX11ConstantBuffer.h"

namespace Epoch
{
	std::shared_ptr<ConstantBuffer> ConstantBuffer::Create(void* aData, uint64_t aSize)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::DirectX11: return std::make_shared<DX11ConstantBuffer>(aData, aSize);
		}
		EPOCH_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	std::shared_ptr<ConstantBuffer> ConstantBuffer::Create(uint64_t aSize)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::DirectX11: return std::make_shared<DX11ConstantBuffer>(aSize);
		}
		EPOCH_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}