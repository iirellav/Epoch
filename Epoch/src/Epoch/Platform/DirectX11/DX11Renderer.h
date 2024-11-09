#pragma once
#include "memory"
#include "Epoch/Rendering/RHI.h"
#include "Epoch/Rendering/RendererAPI.h"

namespace Epoch
{
	class DX11Renderer : public RendererAPI
	{
	public:
		void Init() override;
		void Shutdown() override;

		void BeginFrame() override;
		void EndFrame() override;
		
		void SetComputePipeline(std::shared_ptr<ComputePipeline> aComputePipeline);
		void RemoveComputePipeline(std::shared_ptr<ComputePipeline> aComputePipeline);
		void DispatchCompute(const CU::Vector3ui& aWorkGroups);

		void SetRenderPipeline(std::shared_ptr<RenderPipeline> aRenderPipeline);
		void RemoveRenderPipeline(std::shared_ptr<RenderPipeline> aRenderPipeline);

		void RenderInstancedMesh(std::shared_ptr<Mesh> aMesh, uint32_t aSubmeshIndex, std::shared_ptr<VertexBuffer> aTransformBuffer, uint32_t aInstanceCount) override;
		void RenderAnimatedMesh(std::shared_ptr<Mesh> aMesh) override;
		void RenderQuad() override;
		void RenderGeometry(std::shared_ptr<VertexBuffer> aVertexBuffer, std::shared_ptr<IndexBuffer> aIndexBuffer, uint32_t aIndexCount = 0) override;

		std::pair<std::shared_ptr<TextureCube>, std::shared_ptr<TextureCube>> CreateEnvironmentTextures(const std::string& aFilepath) override;

	private:
		ComPtr<ID3D11SamplerState> wrapSamplerState;
		ComPtr<ID3D11SamplerState> clampSamplerState;
		ComPtr<ID3D11SamplerState> brdfLUTSamplerState;
	};
}
