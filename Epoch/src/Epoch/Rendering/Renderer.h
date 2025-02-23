#pragma once
#include <memory>
#include <CommonUtilities/Math/Matrix/Matrix4x4.hpp>
#include "RendererConfig.h"
#include "Epoch/Utils/FileSystem.h"

namespace Epoch
{
	class ShaderLibrary;
	class RenderPipeline;
	class ComputePipeline;
	class VertexBuffer;
	class IndexBuffer;
	class Mesh;
	class Texture2D;
	class TextureCube;

	class Renderer
	{
	public:
		Renderer() = delete;
		~Renderer() = delete;

		static void Init(const RendererConfig& aRendererConfig);
		static void Shutdown();
		
		static std::shared_ptr<ShaderLibrary> GetShaderLibrary();

		static void BeginFrame();
		static void EndFrame();
		
		static void SetComputePipeline(std::shared_ptr<ComputePipeline> aComputePipeline);
		static void RemoveComputePipeline(std::shared_ptr<ComputePipeline> aComputePipeline);
		static void DispatchCompute(const CU::Vector3ui& aWorkGroups);

		static void SetRenderPipeline(std::shared_ptr<RenderPipeline> aRenderPipeline);
		static void RemoveRenderPipeline(std::shared_ptr<RenderPipeline> aRenderPipeline);

		static void RenderInstancedMesh(std::shared_ptr<Mesh> aMesh, uint32_t aSubmeshIndex, std::shared_ptr<VertexBuffer> aTransformBuffer, uint32_t aInstanceCount);
		static void RenderAnimatedMesh(std::shared_ptr<Mesh> aMesh);
		static void RenderQuad();
		static void RenderGeometry(std::shared_ptr<VertexBuffer> aVertexBuffer, std::shared_ptr<IndexBuffer> aIndexBuffer, uint32_t aIndexCount = 0);

		static std::pair<std::shared_ptr<TextureCube>, std::shared_ptr<TextureCube>> CreateEnvironmentTextures(const std::string& aFilepath);
		static std::pair<std::shared_ptr<TextureCube>, std::shared_ptr<TextureCube>> CreateEnvironmentTextures(std::shared_ptr<Texture2D> aEquirectangularTexture, const std::string& aName = "");

		static std::shared_ptr<Texture2D> GetWhiteTexture();
		static std::shared_ptr<Texture2D> GetBlackTexture();
		static std::shared_ptr<Texture2D> GetFlatNormalTexture();
		static std::shared_ptr<Texture2D> GetDefaultMaterialTexture();
		static std::shared_ptr<TextureCube> GetDefaultBlackCubemap();
		static std::shared_ptr<TextureCube> GetDefaultWhiteCubemap();
		static std::shared_ptr<Texture2D> GetDefaultColorGradingLut();
		static std::shared_ptr<Texture2D> GetBRDFLut();

	private:
		static void OnShaderFileChanged(const std::filesystem::path& aFilepath, FilewatchEvent aEventType);
	};
}
