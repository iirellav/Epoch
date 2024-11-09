#pragma once
#include <string>
#include <memory>
#include "Epoch/Rendering/FrameBuffer.h"
#include "Epoch/Rendering/VertexBuffer.h"
#include "Epoch/Rendering/Shader.h"

namespace Epoch
{
	enum class PrimitiveTopology
	{
		PointList,
		LineList,
		TriangleList
	};

	enum class RasterizerState
	{
		CullBack,
		CullNone,
		CullFront,
		Wireframe
	};

	enum class BlendMode
	{
		OneZero,
		Alpha,
		Additive
	};

	enum class DepthCompareOperator
	{
		Never,
		NotEqual,
		Less,
		LessOrEqual,
		Greater,
		GreaterOrEqual,
		Equal,
		Always
	};

	struct PipelineSpecification
	{
		std::shared_ptr<Shader> shader;
		std::shared_ptr<Framebuffer> targetFramebuffer;

		std::vector<VertexBufferLayout> vertexLayouts;

		PrimitiveTopology primitiveTopology = PrimitiveTopology::TriangleList;
		RasterizerState rasterizerState = RasterizerState::CullBack;
		BlendMode blendMode = BlendMode::OneZero;
		DepthCompareOperator depthCompareOperator = DepthCompareOperator::LessOrEqual;

		bool depthTest = true;
		bool depthWrite = true;

		std::string debugName = "Unnamed";

		PipelineSpecification() = default;
		PipelineSpecification(const std::string& aDebugName) : debugName(aDebugName) {}
	};

	class RenderPipeline
	{
	public:
		virtual ~RenderPipeline() = default;

		PipelineSpecification& GetSpecification() { return mySpecification; }
		const PipelineSpecification& GetSpecification() const { return mySpecification; }
		
		static std::shared_ptr<RenderPipeline> Create(const PipelineSpecification& aSpec);

	protected:
		PipelineSpecification mySpecification;
	};
}
