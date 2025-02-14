#include "epch.h"
#include "SceneRenderer2D.h"
#include "Epoch/Rendering/Renderer.h"
#include "Epoch/Rendering/VertexBuffer.h"
#include "Epoch/Rendering/IndexBuffer.h"
#include "Epoch/Rendering/ConstantBuffer.h"
#include "Epoch/Rendering/RenderPipeline.h"
#include "Epoch/Rendering/Texture.h"

#include "Epoch/Rendering/RHI.h"

namespace Epoch
{
	SceneRenderer2D::SceneRenderer2D()
	{
	}

	SceneRenderer2D::~SceneRenderer2D()
	{
		Shutdown();
	}

	void SceneRenderer2D::Init(std::shared_ptr<Framebuffer> aTargetBuffer)
	{
		EPOCH_PROFILE_FUNC();

		//Quad Data/Buffers
		{
			myQuadUVCoords[0] = { 0.0f, 1.0f };
			myQuadUVCoords[1] = { 0.0f, 0.0f };
			myQuadUVCoords[2] = { 1.0f, 0.0f };
			myQuadUVCoords[3] = { 1.0f, 1.0f };

			myScreenSpaceQuadVertexPositions[0] = { 0.0f, 0.0f };
			myScreenSpaceQuadVertexPositions[1] = { 0.0f, 1.0f };
			myScreenSpaceQuadVertexPositions[2] = { 1.0f, 1.0f };
			myScreenSpaceQuadVertexPositions[3] = { 1.0f, 0.0f };
			
			myQuadVertexPositions[0] = { -50.0f, -50.0f, 0.0f, 1.0f };
			myQuadVertexPositions[1] = { -50.0f,  50.0f, 0.0f, 1.0f };
			myQuadVertexPositions[2] = { 50.0f,  50.0f, 0.0f, 1.0f };
			myQuadVertexPositions[3] = { 50.0f, -50.0f, 0.0f, 1.0f };

			myQuadVertexBuffer = VertexBuffer::Create(MaxQuadVertices, sizeof(QuadVertex));

			uint32_t* quadIndices = new uint32_t[MaxQuadIndices];
			uint32_t offset = 0;
			for (uint32_t i = 0; i < MaxQuadIndices; i += 6)
			{
				quadIndices[i + 0] = offset + 0;
				quadIndices[i + 1] = offset + 1;
				quadIndices[i + 2] = offset + 2;

				quadIndices[i + 3] = offset + 2;
				quadIndices[i + 4] = offset + 3;
				quadIndices[i + 5] = offset + 0;

				offset += 4;
			}

			myQuadIndexBuffer = IndexBuffer::Create(quadIndices, MaxQuadIndices);
			delete[] quadIndices;
		}

		myCameraBuffer = ConstantBuffer::Create(sizeof(CU::Matrix4x4f));

		//Quads
		{
			VertexBufferLayout layout =
			{
				{ ShaderDataType::Float3,	"POSITION" },
				{ ShaderDataType::UInt,		"ID" },
				{ ShaderDataType::Float4,	"TINT" },
				{ ShaderDataType::Float2,	"UV" }
			};

			FramebufferSpecification specs;
			specs.existingFramebuffer = aTargetBuffer;
			//specs.attachments = { { TextureFormat::RGBA, "Color" } };
			//specs.existingColorAttachments.emplace(0, aTargetBuffer->GetTarget());
			specs.clearColorOnLoad = false;
			specs.clearDepthOnLoad = false;

			PipelineSpecification pipelineSpecs("Quads");
			pipelineSpecs.targetFramebuffer = Framebuffer::Create(specs);
			pipelineSpecs.shader = Renderer::GetShaderLibrary()->Get("Sprite");
			pipelineSpecs.vertexLayouts.push_back(layout);
			myQuadPipeline = RenderPipeline::Create(pipelineSpecs);
		}
	}

	void SceneRenderer2D::Shutdown()
	{
	}

	void SceneRenderer2D::SetViewportSize(uint32_t aWidth, uint32_t aHeight)
	{
		if ((myViewportWidth != aWidth || myViewportHeight != aHeight) && aWidth > 0 && aHeight > 0)
		{
			myViewportWidth = aWidth;
			myViewportHeight = aHeight;
			myNeedsResize = true;
		}
	}

	void SceneRenderer2D::BeginScene(const CU::Matrix4x4f& aProj, const CU::Matrix4x4f& aView)
	{
		EPOCH_PROFILE_FUNC();

		EPOCH_ASSERT(!myActive, "Can't call begin scene while rendering!");
		myActive = true;

		myProj = aProj;
		myView = aView;
		myViewProj = aView * aProj;

		if (myNeedsResize)
		{
			myNeedsResize = false;

			myQuadPipeline->GetSpecification().targetFramebuffer->Resize(myViewportWidth, myViewportHeight);
		}

		myCameraBuffer->SetData(&myViewProj);
		myCameraBuffer->Bind(PIPELINE_STAGE_VERTEX_SHADER, 0);
	}

	void SceneRenderer2D::EndScene()
	{
		EPOCH_PROFILE_FUNC();

		EPOCH_ASSERT(myActive, "Can't call end scene if not rendering!");
		myActive = false;

		QuadPass();

		myQuadVertices.clear();
		myTextures.clear();
	}

	static CU::Vector2f FlipUVCoord(const CU::Vector2f& aCoord, bool aFlipX, bool aFlipY)
	{
		return { CU::Math::Abs(aCoord.x - (float)aFlipX), CU::Math::Abs(aCoord.y - (float)aFlipY) };
	}

	void SceneRenderer2D::SubmitQuad(const CU::Matrix4x4f aTransform, std::shared_ptr<Texture2D> aTexture, const QuadSetting& aSettings, uint32_t aEntityID)
	{
		CU::Vector4f scaleMultiplier = CU::Vector4f::One;
		if (aTexture)
		{

			if ((float)aTexture->GetWidth() < (float)aTexture->GetHeight())
			{
				scaleMultiplier.y = (float)aTexture->GetHeight() / (float)aTexture->GetWidth();
			}
			else
			{
				scaleMultiplier.x = (float)aTexture->GetWidth() / (float)aTexture->GetHeight();
			}
		}

		if (!aTexture) aTexture = Renderer::GetWhiteTexture();
		auto& vertexList = myQuadVertices[aTexture->GetHandle()];
		myTextures[aTexture->GetHandle()] = aTexture;

		for (size_t i = 0; i < 4; i++)
		{
			QuadVertex& vertex = vertexList.emplace_back();
			vertex.position = aTransform * (myQuadVertexPositions[i] * scaleMultiplier);
			vertex.uv = FlipUVCoord(myQuadUVCoords[i], aSettings.flipX, aSettings.flipY);
			vertex.tint = aSettings.tint.GetVector4();
			vertex.entityID = aEntityID;
		}
	}
	
	void SceneRenderer2D::SubmitScreenSpaceQuad(const CU::Matrix4x4f aTransform, const CU::Vector2ui aSize, std::shared_ptr<Texture2D> aTexture, const ScreenSpaceQuadSetting& aSettings, uint32_t aEntityID)
	{
		if (!aTexture) aTexture = Renderer::GetWhiteTexture();
		auto& vertexList = myQuadVertices[aTexture->GetHandle()];
		myTextures[aTexture->GetHandle()] = aTexture;

		const CU::Vector4f anchorOffset = { myViewportWidth * aSettings.anchor.x, myViewportHeight * aSettings.anchor.y, 0.0f, 0.0f };
		const CU::Vector2f size = { (float)aSize.x, (float)aSize.y };

		for (size_t i = 0; i < 4; i++)
		{
			const CU::Vector2f vertPos = (myScreenSpaceQuadVertexPositions[i] - aSettings.pivot) * size;

			QuadVertex& vertex = vertexList.emplace_back();
			vertex.position = aTransform * CU::Vector4f(vertPos.x, vertPos.y, 0.0f, 1.0f) + anchorOffset;
			vertex.uv = FlipUVCoord(myQuadUVCoords[i], aSettings.flipX, aSettings.flipY);
			vertex.tint = aSettings.tint.GetVector4();
			vertex.entityID = aEntityID;
		}
	}

	void SceneRenderer2D::QuadPass()
	{
		Renderer::SetRenderPipeline(myQuadPipeline);

		for (const auto& [texture, vertexList] : myQuadVertices)
		{
			if (!myTextures[texture])
			{
				continue;
			}

			std::vector<ID3D11ShaderResourceView*> SRVs(1);
			auto dxTexture = std::dynamic_pointer_cast<DX11Texture2D>(myTextures[texture]);
			SRVs[0] = dxTexture->GetSRV().Get();
			RHI::GetContext()->PSSetShaderResources(0, 1, SRVs.data());

			const uint32_t quadCount = (uint32_t)vertexList.size() / 4;
			for (uint32_t i = 0; i < quadCount; i += MaxQuads)
			{
				uint32_t count = CU::Math::Min(quadCount - i, MaxQuads);

				myQuadVertexBuffer->SetData((void*)vertexList.data(), count * 4, i * 4 * sizeof(QuadVertex));
				Renderer::RenderGeometry(myQuadVertexBuffer, myQuadIndexBuffer, count * 6);
			}

			std::vector<ID3D11ShaderResourceView*> emptySRVs(1);
			RHI::GetContext()->PSSetShaderResources(0, 1, emptySRVs.data());
		}

		Renderer::RemoveRenderPipeline(myQuadPipeline);
	}
}