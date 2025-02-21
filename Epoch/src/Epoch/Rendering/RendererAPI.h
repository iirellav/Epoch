#pragma once
#include <string>

namespace Epoch
{
	class RenderPipeline;
	class ComputePipeline;
	class VertexBuffer;
	class IndexBuffer;
	class Mesh;
	class Texture2D;
	class TextureCube;

	enum class RendererAPIType
	{
		DirectX11,
		DirectX12,
	};

	class RendererAPI
	{
	public:
		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;
		
		virtual void SetComputePipeline(std::shared_ptr<ComputePipeline> aRenderPipeline) = 0;
		virtual void RemoveComputePipeline(std::shared_ptr<ComputePipeline> aRenderPipeline) = 0;
		virtual void DispatchCompute(const CU::Vector3ui& aWorkGroups) = 0;

		virtual void SetRenderPipeline(std::shared_ptr<RenderPipeline> aRenderPipeline) = 0;
		virtual void RemoveRenderPipeline(std::shared_ptr<RenderPipeline> aRenderPipeline) = 0;

		virtual void RenderInstancedMesh(std::shared_ptr<Mesh> aMesh, uint32_t aSubmeshIndex, std::shared_ptr<VertexBuffer> aTransformBuffer, uint32_t aInstanceCount) = 0;
		virtual void RenderAnimatedMesh(std::shared_ptr<Mesh> aMesh) = 0;
		virtual void RenderQuad() = 0;
		virtual void RenderGeometry(std::shared_ptr<VertexBuffer> aVertexBuffer, std::shared_ptr<IndexBuffer> aIndexBuffer, uint32_t aIndexCount = 0) = 0;
		
		virtual std::pair<std::shared_ptr<TextureCube>, std::shared_ptr<TextureCube>> CreateEnvironmentTextures(const std::string& aFilepath) = 0;
		virtual std::pair<std::shared_ptr<TextureCube>, std::shared_ptr<TextureCube>> CreateEnvironmentTextures(std::shared_ptr<Texture2D> aEquirectangularTexture, const std::string& aName = "") = 0;

		static RendererAPIType Current() { return staticCurrentRendererAPI; }
		static std::string CurrentName();
		static void SetAPI(RendererAPIType aApi);

	private:
		inline static RendererAPIType staticCurrentRendererAPI = RendererAPIType::DirectX11;
	};
}
