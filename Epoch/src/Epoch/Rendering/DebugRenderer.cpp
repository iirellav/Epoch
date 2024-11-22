#include "epch.h"
#include "DebugRenderer.h"
#include <CommonUtilities/Math/Transform.h>
#include <CommonUtilities/Math/CommonMath.hpp>
#include "Epoch/Debug/Instrumentor.h"
#include "Epoch/Rendering/Renderer.h"
#include "Epoch/Rendering/VertexBuffer.h"
#include "Epoch/Rendering/IndexBuffer.h"
#include "Epoch/Rendering/RenderPipeline.h"
#include "Epoch/Rendering/ConstantBuffer.h"
#include "Epoch/Rendering/Texture.h"

#include "RHI.h" //TEMP

namespace Epoch
{
	DebugRenderer::~DebugRenderer()
	{
		Shutdown();
	}

	void DebugRenderer::Init(std::shared_ptr<Framebuffer> aTargetBuffer)
	{
		EPOCH_PROFILE_FUNC();

		myVertexBuffer = VertexBuffer::Create(MaxLineVertices, sizeof(LineVertex));
		myVertices.reserve(MaxLineVertices);

		myIndexBuffer = IndexBuffer::Create(MaxLineIndices);
		myIndices.reserve(MaxLineIndices);

		//Lines
		{
			VertexBufferLayout layout =
			{
				{ ShaderDataType::Float3,	"POSITION" },
				{ ShaderDataType::Float4,	"COLOR" }
			};
			
			FramebufferSpecification specs;
			specs.existingFramebuffer = aTargetBuffer;
			specs.clearColorOnLoad = false;
			specs.clearDepthOnLoad = false;

			PipelineSpecification spec("Debug Lines");
			spec.shader = Renderer::GetShaderLibrary()->Get("DebugWireframe");
			spec.targetFramebuffer = Framebuffer::Create(specs);
			spec.vertexLayouts.push_back(layout);
			spec.primitiveTopology = PrimitiveTopology::LineList;
			spec.rasterizerState = RasterizerState::Wireframe;
			myLinePipelineState = RenderPipeline::Create(spec);

			spec.depthCompareOperator = DepthCompareOperator::Greater;
			myOccludedLinePipelineState = RenderPipeline::Create(spec);
		}

		//Grid
		{
			FramebufferSpecification specs;
			specs.existingFramebuffer = aTargetBuffer;
			specs.clearColorOnLoad = false;
			specs.clearDepthOnLoad = false;

			PipelineSpecification spec("Grid");
			spec.shader = Renderer::GetShaderLibrary()->Get("Grid");
			spec.targetFramebuffer = Framebuffer::Create(specs);
			spec.rasterizerState = RasterizerState::CullNone;
			spec.blendMode = BlendMode::Alpha;
			myGridPipelineState = RenderPipeline::Create(spec);
		}

		// Quads
		{
			myTextureSlots[0] = Renderer::GetWhiteTexture();

			myQuadUVCoords[0] = { 0.0f, 1.0f };
			myQuadUVCoords[1] = { 0.0f, 0.0f };
			myQuadUVCoords[2] = { 1.0f, 0.0f };
			myQuadUVCoords[3] = { 1.0f, 1.0f };

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

			VertexBufferLayout layout =
			{
				{ ShaderDataType::Float3,	"POSITION" },
				{ ShaderDataType::UInt,		"TEXINDEX" },
				{ ShaderDataType::Float4,	"TINT" },
				{ ShaderDataType::Float2,	"UV" }
			};

			FramebufferSpecification specs;
			specs.existingFramebuffer = aTargetBuffer;
			specs.clearColorOnLoad = false;
			specs.clearDepthOnLoad = false;

			PipelineSpecification pipelineSpecs("Billboard Quads");
			pipelineSpecs.targetFramebuffer = Framebuffer::Create(specs);
			pipelineSpecs.shader = Renderer::GetShaderLibrary()->Get("Sprite");
			pipelineSpecs.vertexLayouts.push_back(layout);
			pipelineSpecs.blendMode = BlendMode::Alpha;
			pipelineSpecs.rasterizerState = RasterizerState::CullNone;
			myQuadPipelineState = RenderPipeline::Create(pipelineSpecs);
		}

		myCameraBuffer = ConstantBuffer::Create(sizeof(CameraBuffer));
		myDebugLineBuffer = ConstantBuffer::Create(sizeof(CU::Color));
		myGridBuffer = ConstantBuffer::Create(sizeof(GridBuffer));
	}

	void DebugRenderer::SetViewportSize(unsigned aWidth, unsigned aHeight)
	{
		myLinePipelineState->GetSpecification().targetFramebuffer->Resize(aWidth, aHeight);
		myGridPipelineState->GetSpecification().targetFramebuffer->Resize(aWidth, aHeight);
		myOccludedLinePipelineState->GetSpecification().targetFramebuffer->Resize(aWidth, aHeight);
		myQuadPipelineState->GetSpecification().targetFramebuffer->Resize(aWidth, aHeight);
	}

	void DebugRenderer::Shutdown()
	{
	}

	void DebugRenderer::Render(const CU::Matrix4x4f& aView, const CU::Matrix4x4f& aProjection)
	{
		EPOCH_PROFILE_FUNC();

		myStats = Stats();

		CameraBuffer camBuffer(aView * aProjection);
		myCameraBuffer->SetData(&camBuffer);
		myCameraBuffer->Bind(PIPELINE_STAGE_VERTEX_SHADER, 0);

		for (size_t i = 0; i < 2; i++)
		{
			if (i == 0)
			{
				CU::Color tint = CU::Color::White;
				myDebugLineBuffer->SetData(&tint);
				myDebugLineBuffer->Bind(PIPELINE_STAGE_PIXEL_SHADER, 0);
				Renderer::SetRenderPipeline(myLinePipelineState);
			}
			else
			{
				CU::Color tint = CU::Color::White * 0.3f;
				tint.a = 1.0f;
				myDebugLineBuffer->SetData(&tint);
				myDebugLineBuffer->Bind(PIPELINE_STAGE_PIXEL_SHADER, 0);
				Renderer::SetRenderPipeline(myOccludedLinePipelineState);
			}

			uint32_t vxOffset = 0;
			uint32_t ixOffset = 0;
			for (size_t i = 0; i < myFrames.size(); i++)
			{
				const Frame& frame = myFrames[i];

				if (vxOffset + frame.vertexCount >= MaxLineVertices || ixOffset + frame.indexCount >= MaxLineIndices)
				{
					Flush();
					vxOffset = 0;
					ixOffset = 0;
					--i;
				}
				else
				{
					for (size_t v = 0; v < frame.vertexCount; v++)
					{
						myVertices.push_back(frame.vertices[v]);
					}

					for (size_t v = 0; v < frame.indexCount; v++)
					{
						myIndices.emplace_back(frame.indices[v] + vxOffset);
					}

					vxOffset += frame.vertexCount;
					ixOffset += frame.indexCount;
				}
			}

			if (vxOffset != 0)
			{
				Flush();
				vxOffset = 0;
				ixOffset = 0;
			}

			if (i == 0)
			{
				Renderer::RemoveRenderPipeline(myLinePipelineState);
			}
			else
			{
				Renderer::RemoveRenderPipeline(myOccludedLinePipelineState);
			}
		}

		//Quads
		{
			Renderer::SetRenderPipeline(myQuadPipelineState);

			uint32_t textureCount = 0;
			std::vector<ID3D11ShaderResourceView*> SRVs;
			SRVs.reserve(MaxTextureSlots);
			for (size_t i = 0; i < MaxTextureSlots; i++)
			{
				if (myTextureSlots[i])
				{
					SRVs.push_back((ID3D11ShaderResourceView*)myTextureSlots[i]->GetView());
					++textureCount;
				}
				else
				{
					break;
				}
			}

			RHI::GetContext()->PSSetShaderResources(0, textureCount, SRVs.data());

			for (uint32_t i = 0; i < myQuadCount; i += MaxQuads)
			{
				uint32_t count = CU::Math::Min(myQuadCount - i, MaxQuads);

				myQuadVertexBuffer->SetData(myQuadVertices.data(), count * 4, i * 4 * sizeof(QuadVertex));
				Renderer::RenderGeometry(myQuadVertexBuffer, myQuadIndexBuffer, count * 6);
			}

			std::vector<ID3D11ShaderResourceView*> emptySRVs(textureCount);
			RHI::GetContext()->PSSetShaderResources(0, textureCount, emptySRVs.data());

			Renderer::RemoveRenderPipeline(myQuadPipelineState);
		}

		//Quad Stats
		{
			myStats.drawCalls += CU::Math::CeilToUInt((float)myQuadCount / MaxQuads);
			myStats.vertices += myQuadCount * 4;
			myStats.indices += myQuadCount * 6;
		}

		myFrames.clear();

		myQuadVertices.clear();
		myQuadCount = 0;

		for (size_t i = 1; i < MaxTextureSlots; i++)
		{
			myTextureSlots[i] = nullptr;
		}
	}

	void DebugRenderer::Flush()
	{
		EPOCH_PROFILE_FUNC();

		++myStats.drawCalls;
		myStats.vertices += (uint32_t)myVertices.size();
		myStats.indices += (uint32_t)myIndices.size();

		myVertexBuffer->SetData(myVertices.data(), (uint32_t)myVertices.size());
		myIndexBuffer->SetData(myIndices.data(), (uint32_t)myIndices.size());

		Renderer::RenderGeometry(myVertexBuffer, myIndexBuffer);


		myVertices.clear();
		myIndices.clear();
	}

	void DebugRenderer::DrawLine(const CU::Vector3f& aP0, const CU::Vector3f& aP1, CU::Color aColor)
	{
		auto& frame = myFrames.emplace_back();

		frame.vertices.push_back({ aP0, aColor });
		frame.vertices.push_back({ aP1, aColor });

		frame.indices.push_back(0);
		frame.indices.push_back(1);

		frame.vertexCount = (uint32_t)frame.vertices.size();
		frame.indexCount = (uint32_t)frame.indices.size();
	}

	void DebugRenderer::DrawCircle(const CU::Vector3f& aPosition, const CU::Vector3f& aRotation, float aRadius, const CU::Color aColor)
	{
		auto& frame = myFrames.emplace_back();

		CU::Matrix4x4f trans = CU::Transform(aPosition, aRotation).GetMatrix();

		CU::Vector3f pos;
		const int vertexCount = 32;
		const float segRotationAngle = (360.0f / vertexCount) * CU::Math::ToRad;
		for (int i = 1; i < vertexCount + 1; i++)
		{
			float finalSegRotationAngle = ((float)i * segRotationAngle);

			pos.x = aRadius * cosf(finalSegRotationAngle);
			pos.y = aRadius * sinf(finalSegRotationAngle);

			frame.vertices.push_back({ trans * CU::Vector4f(pos, 1.0f), aColor });
		}

		for (int i = 0; i < vertexCount - 1; i++)
		{
			frame.indices.push_back(i);
			frame.indices.push_back(i + 1);
		}

		frame.indices.push_back(vertexCount - 1);
		frame.indices.push_back(0);

		frame.vertexCount = (uint32_t)frame.vertices.size();
		frame.indexCount = (uint32_t)frame.indices.size();
	}

	void DebugRenderer::DrawWireBox(const CU::Vector3f& aPosition, const CU::Vector3f& aRotation, const CU::Vector3f& aExtent, const CU::Color aColor)
	{
		auto& frame = myFrames.emplace_back();

		CU::Matrix4x4f trans = CU::Transform(aPosition, aRotation, aExtent).GetMatrix();

		frame.vertices.push_back({ { trans * CU::Vector4f(-1.0f,1.0f,1.0f, 1.0f) }, aColor });
		frame.vertices.push_back({ { trans * CU::Vector4f(1.0f,1.0f,1.0f, 1.0f) }, aColor });
		frame.vertices.push_back({ { trans * CU::Vector4f(1.0f,1.0f, -1.0f, 1.0f) }, aColor });
		frame.vertices.push_back({ { trans * CU::Vector4f(-1.0f,1.0f, -1.0f, 1.0f) }, aColor });

		frame.vertices.push_back({ { trans * CU::Vector4f(-1.0f, -1.0f,1.0f, 1.0f) }, aColor });
		frame.vertices.push_back({ { trans * CU::Vector4f(1.0f, -1.0f,1.0f, 1.0f) }, aColor });
		frame.vertices.push_back({ { trans * CU::Vector4f(1.0f, -1.0f, -1.0f, 1.0f) }, aColor });
		frame.vertices.push_back({ { trans * CU::Vector4f(-1.0f, -1.0f, -1.0f, 1.0f) }, aColor });


		frame.indices.push_back(0);
		frame.indices.push_back(1);

		frame.indices.push_back(1);
		frame.indices.push_back(2);

		frame.indices.push_back(2);
		frame.indices.push_back(3);

		frame.indices.push_back(3);
		frame.indices.push_back(0);


		frame.indices.push_back(4);
		frame.indices.push_back(5);

		frame.indices.push_back(5);
		frame.indices.push_back(6);

		frame.indices.push_back(6);
		frame.indices.push_back(7);

		frame.indices.push_back(7);
		frame.indices.push_back(4);


		frame.indices.push_back(0);
		frame.indices.push_back(4);

		frame.indices.push_back(1);
		frame.indices.push_back(5);

		frame.indices.push_back(2);
		frame.indices.push_back(6);

		frame.indices.push_back(3);
		frame.indices.push_back(7);

		frame.vertexCount = (uint32_t)frame.vertices.size();
		frame.indexCount = (uint32_t)frame.indices.size();
	}

	void DebugRenderer::DrawWireSphere(const CU::Vector3f& aPosition, const CU::Vector3f& aRotation, float aRadius, const CU::Color aColor)
	{
		auto& frame = myFrames.emplace_back();

		const CU::Matrix3x3f rotationMatrix = CU::Quatf(aRotation).GetRotationMatrix3x3();
		CU::Vector3f pos;
		const int vertexCount = 32;
		const float segRotationAngle = (360.0f / vertexCount) * CU::Math::ToRad;

		for (int i = 0; i < 3; i++)
		{
			CU::Matrix4x4f trans;
			CU::Matrix4x4f rotationOffsetMatrix;

			if (i == 0)
			{
				trans = CU::Transform(aPosition, aRotation).GetMatrix();
			}
			else if (i == 1)
			{
				trans = CU::Transform(aPosition, aRotation).GetMatrix();
				rotationOffsetMatrix = CU::Quatf(0.0f, CU::Math::DegToRad(90.0f), 0.0f).GetRotationMatrix4x4f();
			}
			else if (i == 2)
			{
				trans = CU::Transform(aPosition, aRotation).GetMatrix();
				rotationOffsetMatrix = CU::Quatf(CU::Math::DegToRad(90.0f), 0.0f, 0.0f).GetRotationMatrix4x4f();
			}

			for (int j = 1; j < vertexCount + 1; j++)
			{
				float finalSegRotationAngle = ((float)j * segRotationAngle);

				pos.x = aRadius * cosf(finalSegRotationAngle);
				pos.y = aRadius * sinf(finalSegRotationAngle);

				frame.vertices.push_back({ trans * (rotationOffsetMatrix * CU::Vector4f(pos, 1.0f)), aColor });
			}

			for (int j = 0; j < vertexCount - 1; j++)
			{
				frame.indices.push_back(vertexCount * i + j);
				frame.indices.push_back(vertexCount * i + j + 1);
			}

			frame.indices.push_back(vertexCount * i + vertexCount - 1);
			frame.indices.push_back(vertexCount * i + 0);
		}

		frame.vertexCount = (uint32_t)frame.vertices.size();
		frame.indexCount = (uint32_t)frame.indices.size();
	}

	void DebugRenderer::DrawWireCapsule(const CU::Vector3f& aPosition, const CU::Vector3f& aRotation, float aRadius, float aHeight, const CU::Color aColor)
	{
		Frame frame;

		const CU::Matrix3x3f rotationMatrix = CU::Quatf(aRotation).GetRotationMatrix3x3();
		const CU::Vector3f circleRotation = CU::Quatf(CU::Quatf(CU::Math::DegToRad(90.0f), 0.0f, 0.0f).GetRotationMatrix3x3() * rotationMatrix).GetEulerAngles();

		CU::Vector3f pos;
		CU::Vector3f posOffset(0.0f, aHeight * 0.5f - aRadius, 0.0f);
		const float segRotationAngle = (360.0f / 32) * CU::Math::ToRad;

		for (int i = 0; i < 2; i++)
		{
			CU::Matrix4x4f trans;
			CU::Matrix4x4f rotationOffsetMatrix;

			if (i == 0)
			{
				trans = CU::Transform(aPosition, aRotation).GetMatrix();
			}
			else if (i == 1)
			{
				trans = CU::Transform(aPosition, aRotation).GetMatrix();
				rotationOffsetMatrix = CU::Quatf(0.0f, CU::Math::DegToRad(90.0f), 0.0f).GetRotationMatrix4x4f();
			}

			for (int j = 0; j < 17; j++)
			{
				float finalSegRotationAngle = ((float)j * segRotationAngle);

				pos.x = aRadius * cosf(finalSegRotationAngle);
				pos.y = aRadius * sinf(finalSegRotationAngle);

				frame.vertices.push_back({ trans * (rotationOffsetMatrix * CU::Vector4f(pos + posOffset, 1.0f)), aColor });
			}

			for (int j = 0; j < 17 - 1; j++)
			{
				frame.indices.push_back(17 * i + j);
				frame.indices.push_back(17 * i + j + 1);
			}
		}

		CU::Vector3f circlePos = aPosition + rotationMatrix * posOffset;
		DrawCircle(circlePos, circleRotation, aRadius, aColor);

		posOffset *= -1.0f;

		for (int i = 2; i < 4; i++)
		{
			CU::Matrix4x4f trans;
			CU::Matrix4x4f rotationOffsetMatrix;
			if (i == 2)
			{
				trans = CU::Transform(aPosition, aRotation).GetMatrix();
			}
			else if (i == 3)
			{
				trans = CU::Transform(aPosition, aRotation).GetMatrix();
				rotationOffsetMatrix = CU::Quatf(0.0f, CU::Math::DegToRad(90.0f), 0.0f).GetRotationMatrix4x4f();
			}

			for (int j = 16; j < 33; j++)
			{
				float finalSegRotationAngle = ((float)j * segRotationAngle);

				pos.x = aRadius * cosf(finalSegRotationAngle);
				pos.y = aRadius * sinf(finalSegRotationAngle);

				frame.vertices.push_back({ trans * (rotationOffsetMatrix * CU::Vector4f(pos + posOffset, 1.0f)), aColor });
			}

			for (int j = 0; j < 17 - 1; j++)
			{
				frame.indices.push_back(17 * i + j);
				frame.indices.push_back(17 * i + j + 1);
			}
		}

		circlePos = aPosition + rotationMatrix * posOffset;
		DrawCircle(circlePos, circleRotation, aRadius, aColor);

		frame.indices.push_back(0);
		frame.indices.push_back(50);

		frame.indices.push_back(17);
		frame.indices.push_back(67);

		frame.indices.push_back(33);
		frame.indices.push_back(51);

		frame.indices.push_back(16);
		frame.indices.push_back(34);

		frame.vertexCount = (uint32_t)frame.vertices.size();
		frame.indexCount = (uint32_t)frame.indices.size();

		myFrames.push_back(frame);
	}

	void DebugRenderer::DrawWireCone(const CU::Vector3f& aPosition, const CU::Vector3f& aRotation, float aAngle, float aRange, const CU::Color aColor)
	{
		auto& frame = myFrames.emplace_back();

		CU::Matrix4x4f trans = CU::Transform(aPosition, aRotation).GetMatrix();

		float radius = aRange * tanf(aAngle);

		CU::Vector3f pos;
		const int vertexCount = 32;
		const float segRotationAngle = (360.0f / vertexCount) * CU::Math::ToRad;
		for (int i = 1; i < vertexCount + 1; i++)
		{
			float finalSegRotationAngle = ((float)i * segRotationAngle);

			pos.x = radius * cosf(finalSegRotationAngle);
			pos.y = radius * sinf(finalSegRotationAngle);

			pos.z = aRange;

			frame.vertices.push_back({ trans * CU::Vector4f(pos, 1.0f), aColor });
		}

		for (int i = 0; i < vertexCount - 1; i++)
		{
			frame.indices.push_back(i);
			frame.indices.push_back(i + 1);
		}

		frame.indices.push_back(vertexCount - 1);
		frame.indices.push_back(0);

		for (int i = 0; i < 4; i++)
		{
			frame.vertices.push_back({ trans * CU::Vector4f(0, 0, 0, 1.0f), aColor });

			frame.indices.push_back(vertexCount);
			frame.indices.push_back(vertexCount / 4 * i);
		}

		frame.vertexCount = (uint32_t)frame.vertices.size();
		frame.indexCount = (uint32_t)frame.indices.size();
	}

	void DebugRenderer::DrawGrid(const CU::Vector3f& aPosition, const CU::Vector3f& aRotation, const CU::Vector2i& aSize, float aAlpha)
	{
		EPOCH_PROFILE_FUNC();

		CU::Transform trans(aPosition, aRotation * CU::Math::ToRad);
		GridBuffer gridBuffer(trans.GetMatrix(), { (float)aSize.x, (float)aSize.y }, aAlpha);
		myGridBuffer->SetData(&gridBuffer);
		myGridBuffer->Bind(PIPELINE_STAGE_VERTEX_SHADER | PIPELINE_STAGE_PIXEL_SHADER, 1);

		Renderer::SetRenderPipeline(myGridPipelineState);
		Renderer::RenderQuad();
		Renderer::RemoveRenderPipeline(myGridPipelineState);
	}

	void DebugRenderer::DrawBillboardedQuad(std::shared_ptr<Texture2D> aTexture, const CU::Vector3f& aPosition, const CU::Vector3f& aTarget, const CU::Vector3f& aUp, const CU::Color& aTint, float aScale)
	{
		CU::Transform quadTrans(aPosition, CU::Vector3f::Zero, CU::Vector3f(aScale));
		quadTrans.LookAt(aPosition + CU::Vector3f(aPosition - aTarget).GetNormalized(), aUp);

		uint32_t textureIndex = 0;
		if (aTexture)
		{
			for (size_t i = 0; i < MaxTextureSlots; i++)
			{
				if (!myTextureSlots[i])
				{
					myTextureSlots[i] = aTexture;
					textureIndex = (uint32_t)i;
					break;
				}
				else if (myTextureSlots[i]->GetHandle() == aTexture->GetHandle())
				{
					textureIndex = (uint32_t)i;
					break;
				}
			}
		}

		for (size_t i = 0; i < 4; i++)
		{
			QuadVertex& vertex = myQuadVertices.emplace_back();
			vertex.position = quadTrans.GetMatrix() * myQuadVertexPositions[i];
			vertex.uv = myQuadUVCoords[i];
			vertex.tint = aTint.GetVector4();
			vertex.texIndex = textureIndex;
		}
		++myQuadCount;
	}
}
