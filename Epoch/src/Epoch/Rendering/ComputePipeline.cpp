#include "epch.h"
#include "ComputePipeline.h"
#include "Epoch/Rendering/RendererAPI.h"
#include "Epoch/Platform/DirectX11/DX11ComputePipeline.h"

namespace Epoch
{
	std::shared_ptr<ComputePipeline> ComputePipeline::Create(const ComputePipelineSpecification& aSpec)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::DirectX11: return std::make_shared<DX11ComputePipeline>(aSpec);
		}
		EPOCH_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}
