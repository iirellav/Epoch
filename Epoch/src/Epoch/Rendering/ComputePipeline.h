#pragma once
#include <string>
#include <memory>
#include "Epoch/Rendering/Shader.h"

namespace Epoch
{
	struct ComputePipelineSpecification
	{
		std::shared_ptr<Shader> shader;
		
		std::string debugName = "Unnamed";

		ComputePipelineSpecification() = default;
		ComputePipelineSpecification(const std::string& aDebugName) : debugName(aDebugName) {}
	};

	class ComputePipeline
	{
	public:
		virtual ~ComputePipeline() = default;
		
		ComputePipelineSpecification& GetSpecification() { return mySpecification; }
		const ComputePipelineSpecification& GetSpecification() const { return mySpecification; }

		static std::shared_ptr<ComputePipeline> Create(const ComputePipelineSpecification& aSpec);

	protected:
		ComputePipelineSpecification mySpecification;
	};
}
