#include "epch.h"
#include "DX11ComputePipeline.h"
#include "DX11Shader.h"

namespace Epoch
{
	DX11ComputePipeline::DX11ComputePipeline(const ComputePipelineSpecification& aSpec)
	{
		mySpecification = aSpec;

		auto dx11Shader = std::dynamic_pointer_cast<DX11Shader>(aSpec.shader);
		if (!dx11Shader->HasShader(ShaderStage::Compute))
		{
			LOG_ERROR("Shader '{}' specified in pipeline state '{}' doesn't have a compute shader", aSpec.shader->GetName(), aSpec.debugName);
			EPOCH_ASSERT(false, "A compute shader has to be provided for the creation of a compute pipeline state");
		}
	}
}
