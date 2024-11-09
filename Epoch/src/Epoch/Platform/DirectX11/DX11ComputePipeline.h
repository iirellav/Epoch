#pragma once
#include "Epoch/Rendering/ComputePipeline.h"

namespace Epoch
{
	class DX11ComputePipeline : public ComputePipeline
	{
	public:
		DX11ComputePipeline(const ComputePipelineSpecification& aSpec);
		~DX11ComputePipeline() override = default;

	private:

	};
}
