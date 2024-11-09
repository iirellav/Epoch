#include "epch.h"
#include "RenderPipeline.h"
#include "Epoch/Rendering/RendererAPI.h"
#include "Epoch/Platform/DirectX11/DX11RenderPipeline.h"

namespace Epoch
{
	std::shared_ptr<RenderPipeline> RenderPipeline::Create(const PipelineSpecification& aSpec)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::DirectX11: return std::make_shared<DX11RenderPipeline>(aSpec);
		}
		EPOCH_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}
