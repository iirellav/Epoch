#include "epch.h"
#include "Renderer.h"
#include "RendererAPI.h"
#include "Epoch/Platform/DirectX11/DX11Renderer.h"
#include "Epoch/Core/Application.h"
#include "Epoch/Rendering/Shader.h"
#include "Epoch/Rendering/Texture.h"
#include "Epoch/Rendering/RenderPipeline.h"

namespace Epoch
{
	struct RendererData
	{
		std::shared_ptr<ShaderLibrary> shaderLibrary;

		std::shared_ptr<Texture2D> whiteTexture;
		std::shared_ptr<Texture2D> blackTexture;
		std::shared_ptr<Texture2D> flatNormalTexture;
		std::shared_ptr<Texture2D> defaultMaterialTexture;
		std::shared_ptr<Texture2D> defaultColorGradingLUT;
		std::shared_ptr<Texture2D> BRDFLUTTexture;
	};

	static RendererData staticRendererData;
	static RendererAPI* staticRendererAPI = nullptr;

	static RendererAPI* InitRendererAPI()
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::DirectX11: return new DX11Renderer();
		}
		EPOCH_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	void Renderer::Init()
	{
		EPOCH_PROFILE_FUNC();

		staticRendererAPI = InitRendererAPI();
		staticRendererAPI->Init();

		staticRendererData.shaderLibrary = std::make_shared<ShaderLibrary>();

		//Shaders
		{
			JobSystem& js = Application::Get().GetJobSystem();

			js.AddAJob([] { staticRendererData.shaderLibrary->Load("Resources/Shaders/BRDF_LUT.hlsl"); });
			js.AddAJob([] { staticRendererData.shaderLibrary->Load("Resources/Shaders/Posterization.hlsl"); });
			js.AddAJob([] { staticRendererData.shaderLibrary->Load("Resources/Shaders/Sprite.hlsl"); });
			js.AddAJob([] { staticRendererData.shaderLibrary->Load("Resources/Shaders/DebugWireframe.hlsl"); });
			js.AddAJob([] { staticRendererData.shaderLibrary->Load("Resources/Shaders/Grid.hlsl"); });
			js.AddAJob([] { staticRendererData.shaderLibrary->Load("Resources/Shaders/EquirectangularToCubeMap.hlsl"); });
			js.AddAJob([] { staticRendererData.shaderLibrary->Load("Resources/Shaders/EnvironmentMipFilter.hlsl"); });
			js.AddAJob([] { staticRendererData.shaderLibrary->Load("Resources/Shaders/CopyTexture.hlsl"); });
			js.AddAJob([] { staticRendererData.shaderLibrary->Load("Resources/Shaders/Text.hlsl"); });
			js.AddAJob([] { staticRendererData.shaderLibrary->Load("Resources/Shaders/GBuffer.hlsl"); });
			js.AddAJob([] { staticRendererData.shaderLibrary->Load("Resources/Shaders/EnvironmentalLight.hlsl"); });
			js.AddAJob([] { staticRendererData.shaderLibrary->Load("Resources/Shaders/PointLight.hlsl"); });
			js.AddAJob([] { staticRendererData.shaderLibrary->Load("Resources/Shaders/Spotlight.hlsl"); });
			js.AddAJob([] { staticRendererData.shaderLibrary->Load("Resources/Shaders/Tonemap.hlsl"); });
			js.AddAJob([] { staticRendererData.shaderLibrary->Load("Resources/Shaders/DebugRender.hlsl"); });
			//js.AddAJob([] { staticRendererData.shaderLibrary->Load("Resources/Shaders/UberShader.hlsl"); });


			staticRendererData.shaderLibrary->Load("Resources/Shaders/UberShader.hlsl");
			
			js.WaitUntilDone();
		}

		//Textures
		{
			TextureSpecification spec;
			spec.format = TextureFormat::RGBA;
			spec.width = 1;
			spec.height = 1;

			constexpr uint32_t whiteTextureData = 0xffffffff;
			spec.debugName = "Default - White";
			staticRendererData.whiteTexture = Texture2D::Create(spec, Buffer(&whiteTextureData, sizeof(uint32_t)));
			spec.debugName = "Default - Black";
			constexpr uint32_t blackTextureData = 0xff000000;
			staticRendererData.blackTexture = Texture2D::Create(spec, Buffer(&blackTextureData, sizeof(uint32_t)));

			constexpr uint32_t flatNormalTextureData = 0xffff7f7f;
			spec.debugName = "Default - Flat Normal";
			staticRendererData.flatNormalTexture = Texture2D::Create(spec, Buffer(&flatNormalTextureData, sizeof(uint32_t)));
			constexpr uint32_t defaultMaterialTextureData = 0xffffffff;
			spec.debugName = "Default - Material";
			staticRendererData.defaultMaterialTexture = Texture2D::Create(spec, Buffer(&defaultMaterialTextureData, sizeof(uint32_t)));

			staticRendererData.defaultColorGradingLUT = Texture2D::Create("Resources/Textures/DefaultColorGradingLUT.png");
		}

		//BRDF_LUT
		{
			FramebufferSpecification specs;
			specs.width = 512;
			specs.height = 512;
			specs.attachments = { { TextureFormat::RG16F, "BRDF_LUT" } };

			PipelineSpecification pipelineSpecs("BRDF LUT");
			pipelineSpecs.targetFramebuffer = Framebuffer::Create(specs);
			pipelineSpecs.shader = Renderer::GetShaderLibrary()->Get("BRDF_LUT");
			auto renderPipeline = RenderPipeline::Create(pipelineSpecs);

			SetRenderPipeline(renderPipeline);
			RenderQuad();
			RemoveRenderPipeline(renderPipeline);

			staticRendererData.BRDFLUTTexture = renderPipeline->GetSpecification().targetFramebuffer->GetTarget();
		}
	}

	void Renderer::Shutdown()
	{
		EPOCH_PROFILE_FUNC();

		staticRendererAPI->Shutdown();
	}

	std::shared_ptr<ShaderLibrary> Renderer::GetShaderLibrary()
	{
		return staticRendererData.shaderLibrary;
	}

	void Renderer::BeginFrame()
	{
	}

	void Renderer::RenderFrame()
	{
	}

	void Renderer::EndFrame()
	{
	}

	void Renderer::SetComputePipeline(std::shared_ptr<ComputePipeline> aComputePipeline)
	{
		staticRendererAPI->SetComputePipeline(aComputePipeline);
	}

	void Renderer::RemoveComputePipeline(std::shared_ptr<ComputePipeline> aComputePipeline)
	{
		staticRendererAPI->RemoveComputePipeline(aComputePipeline);
	}

	void Renderer::DispatchCompute(const CU::Vector3ui& aWorkGroups)
	{
		staticRendererAPI->DispatchCompute(aWorkGroups);
	}

	void Renderer::SetRenderPipeline(std::shared_ptr<RenderPipeline> aRenderPipeline)
	{
		staticRendererAPI->SetRenderPipeline(aRenderPipeline);
	}

	void Renderer::RemoveRenderPipeline(std::shared_ptr<RenderPipeline> aRenderPipeline)
	{
		staticRendererAPI->RemoveRenderPipeline(aRenderPipeline);
	}

	void Renderer::RenderInstancedMesh(std::shared_ptr<Mesh> aMesh, uint32_t aSubmeshIndex, std::shared_ptr<VertexBuffer> aTransformBuffer, uint32_t aInstanceCount)
	{
		staticRendererAPI->RenderInstancedMesh(aMesh, aSubmeshIndex, aTransformBuffer, aInstanceCount);
	}

	void Renderer::RenderAnimatedMesh(std::shared_ptr<Mesh> aMesh)
	{
		staticRendererAPI->RenderAnimatedMesh(aMesh);
	}

	void Renderer::RenderQuad()
	{
		staticRendererAPI->RenderQuad();
	}

	void Renderer::RenderGeometry(std::shared_ptr<VertexBuffer> aVertexBuffer, std::shared_ptr<IndexBuffer> aIndexBuffer, uint32_t aIndexCount)
	{
		staticRendererAPI->RenderGeometry(aVertexBuffer, aIndexBuffer, aIndexCount);
	}

	std::pair<std::shared_ptr<TextureCube>, std::shared_ptr<TextureCube>> Renderer::CreateEnvironmentTextures(const std::string& aFilepath)
	{
		return staticRendererAPI->CreateEnvironmentTextures(aFilepath);
	}

	std::shared_ptr<Texture2D> Renderer::GetWhiteTexture()
	{
		return staticRendererData.whiteTexture;
	}

	std::shared_ptr<Texture2D> Renderer::GetBlackTexture()
	{
		return staticRendererData.blackTexture;
	}

	std::shared_ptr<Texture2D> Renderer::GetFlatNormalTexture()
	{
		return staticRendererData.flatNormalTexture;
	}

	std::shared_ptr<Texture2D> Renderer::GetDefaultMaterialTexture()
	{
		return staticRendererData.defaultMaterialTexture;
	}

	std::shared_ptr<Texture2D> Renderer::GetDefaultColorGradingLut()
	{
		return staticRendererData.defaultColorGradingLUT;
	}

	std::shared_ptr<Texture2D> Renderer::GetBRDFLut()
	{
		return staticRendererData.BRDFLUTTexture;
	}
}