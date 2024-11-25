#include "epch.h"
#include "SceneRenderer.h"
#include <codecvt>
#include "Epoch/Core/Application.h"
#include "Epoch/Rendering/Font.h"
#include "Epoch/Rendering/Mesh.h"
#include "Epoch/Rendering/Material.h"
#include "Epoch/Rendering/Environment.h"
#include "Epoch/Rendering/Texture.h"
#include "Epoch/Rendering/Shader.h"
#include "Epoch/Rendering/Renderer.h"
#include "Epoch/Rendering/DebugRenderer.h"
#include "Epoch/Rendering/VertexBuffer.h"
#include "Epoch/Rendering/IndexBuffer.h"
#include "Epoch/Rendering/ConstantBuffer.h"
#include "Epoch/Rendering/Framebuffer.h"
#include "Epoch/Rendering/RenderPipeline.h"
#include "Epoch/Rendering/ComputePipeline.h"
#include "Epoch/Assets/AssetManager.h"

#include "Epoch/Rendering/RHI.h" //TEMP
#include "Epoch/Scene/Components.h" //TEMP
#include "Epoch/Rendering/MSDFData.h"

namespace Epoch
{
	SceneRenderer::SceneRenderer()
	{
		Init();
	}

	SceneRenderer::~SceneRenderer()
	{
		Shutdown();
	}

	void SceneRenderer::Init()
	{
		EPOCH_PROFILE_FUNC();

		//Quad Data/Buffers
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
		}

		//Text Data/Buffers
		{
			myTextVertexBuffer = VertexBuffer::Create(MaxQuadVertices, sizeof(TextVertex));

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

			myTextIndexBuffer = IndexBuffer::Create(quadIndices, MaxQuadIndices);
			delete[] quadIndices;
		}

		VertexBufferLayout vertexLayout =
		{
			{ ShaderDataType::Float3, "POSITION" },
			{ ShaderDataType::Float3, "NORMAL" },
			{ ShaderDataType::Float3, "TANGENT" },
			{ ShaderDataType::Float2, "UV" },
			{ ShaderDataType::Float3, "COLOR" }
		};

		VertexBufferLayout instanceLayout =
		{
			ShaderDataInputRate::PerInstance,
			{
			{ ShaderDataType::Float4, "ROW", 0 },
			{ ShaderDataType::Float4, "ROW", 1 },
			{ ShaderDataType::Float4, "ROW", 2 },
			{ ShaderDataType::UInt, "ID", 0 },
			}
		};

		VertexBufferLayout boneInfluenceLayout =
		{
			{ ShaderDataType::UInt4,	"BONEIDS" },
			{ ShaderDataType::Float4,	"BONEWEIGHTS" }
		};

		myCameraBuffer = ConstantBuffer::Create(sizeof(CameraBuffer));
		myObjectBuffer = ConstantBuffer::Create(sizeof(ObjectBuffer));
		myMaterialBuffer = ConstantBuffer::Create(sizeof(Material::Data));
		myBoneBuffer = ConstantBuffer::Create(sizeof(BoneBuffer));
		myLightBuffer = ConstantBuffer::Create(sizeof(LightBuffer));
		myPointLightBuffer = ConstantBuffer::Create(sizeof(PointLight) - sizeof(AssetHandle));
		mySpotlightBuffer = ConstantBuffer::Create(sizeof(Spotlight) - sizeof(AssetHandle));
		myDebugDrawModeBuffer = ConstantBuffer::Create(sizeof(CU::Vector4f));
		myPostProcessingBuffer = ConstantBuffer::Create(sizeof(PostProcessingData::BufferData));

		myInstanceTransformBuffer = VertexBuffer::Create(MaxInstanceCount, sizeof(MeshInstanceData));

		//GBuffer
		{
			FramebufferSpecification specs;
			specs.debugName = "GBuffer";
			specs.clearColor = CU::Color::Zero;

			specs.attachments = {
			{ TextureFormat::RGBA,			"Albedo" },
			{ TextureFormat::RGBA,			"Material" },
			{ TextureFormat::RG16F,			"Normal" },
			{ TextureFormat::R11G11B10F,	"Emission" },
			{ TextureFormat::RGBA32F,		"WorldPosition" },
			{ TextureFormat::R32UI,			"EntityID" },
			{ TextureFormat::DEPTH32,		"Depth" } };

			PipelineSpecification pipelineSpecs("GBuffer");
			pipelineSpecs.targetFramebuffer = Framebuffer::Create(specs);
			pipelineSpecs.shader = Renderer::GetShaderLibrary()->Get("GBuffer");
			pipelineSpecs.vertexLayouts.push_back(vertexLayout);
			pipelineSpecs.vertexLayouts.push_back(instanceLayout);
			myGBufferPipeline = RenderPipeline::Create(pipelineSpecs);
		}

		//Environmental Light
		{
			FramebufferSpecification specs;
			specs.debugName = "Scene Buffer";
			specs.attachments = { { TextureFormat::R11G11B10F, "Color" } };

			PipelineSpecification pipelineSpecs("EnvironmentalLight");
			pipelineSpecs.targetFramebuffer = Framebuffer::Create(specs);
			pipelineSpecs.shader = Renderer::GetShaderLibrary()->Get("EnvironmentalLight");
			myEnvironmentalLightPipeline = RenderPipeline::Create(pipelineSpecs);
		}

		//Point Light
		{
			FramebufferSpecification specs;
			specs.existingFramebuffer = myEnvironmentalLightPipeline->GetSpecification().targetFramebuffer;
			specs.clearColorOnLoad = false;
			specs.clearDepthOnLoad = false;

			PipelineSpecification pipelineSpecs("PointLight");
			pipelineSpecs.targetFramebuffer = Framebuffer::Create(specs);
			pipelineSpecs.shader = Renderer::GetShaderLibrary()->Get("PointLight");
			pipelineSpecs.blendMode = BlendMode::Additive;
			myPointLightPipeline = RenderPipeline::Create(pipelineSpecs);
		}

		//Spotlight
		{
			FramebufferSpecification specs;
			specs.existingFramebuffer = myEnvironmentalLightPipeline->GetSpecification().targetFramebuffer;
			specs.clearColorOnLoad = false;
			specs.clearDepthOnLoad = false;

			PipelineSpecification pipelineSpecs("Spotlight");
			pipelineSpecs.targetFramebuffer = Framebuffer::Create(specs);
			pipelineSpecs.shader = Renderer::GetShaderLibrary()->Get("Spotlight");
			pipelineSpecs.blendMode = BlendMode::Additive;
			mySpotlightPipeline = RenderPipeline::Create(pipelineSpecs);
		}

		//Uber
		{
			FramebufferSpecification specs;
			specs.debugName = "Uber";
			specs.attachments = { { TextureFormat::RGBA, "Color" } };

			PipelineSpecification pipelineSpecs("Uber");
			pipelineSpecs.targetFramebuffer = Framebuffer::Create(specs);
			pipelineSpecs.shader = Renderer::GetShaderLibrary()->Get("UberShader");
			pipelineSpecs.depthTest = false;
			pipelineSpecs.depthWrite = false;
			myUberPipeline = RenderPipeline::Create(pipelineSpecs);
		}

		//Sprites
		{
			VertexBufferLayout layout =
			{
				{ ShaderDataType::Float3,	"POSITION" },
				{ ShaderDataType::UInt,		"TEXINDEX" },
				{ ShaderDataType::Float4,	"TINT" },
				{ ShaderDataType::Float2,	"UV" },
				{ ShaderDataType::UInt,		"ID" }
			};

			FramebufferSpecification specs;
			specs.existingAttachments.push_back(myUberPipeline->GetSpecification().targetFramebuffer->GetTarget());
			specs.existingAttachments.push_back(myGBufferPipeline->GetSpecification().targetFramebuffer->GetTarget(5));
			specs.existingDepthAttachment = myGBufferPipeline->GetSpecification().targetFramebuffer->GetDepthAttachment();
			specs.clearColorOnLoad = false;
			specs.clearDepthOnLoad = false;

			PipelineSpecification pipelineSpecs("Sprites");
			pipelineSpecs.targetFramebuffer = Framebuffer::Create(specs);
			pipelineSpecs.shader = Renderer::GetShaderLibrary()->Get("Sprite");
			pipelineSpecs.vertexLayouts.push_back(layout);
			pipelineSpecs.blendMode = BlendMode::Alpha;
			pipelineSpecs.rasterizerState = RasterizerState::CullNone;
			mySpritePipeline = RenderPipeline::Create(pipelineSpecs);
		}

		//Text
		{
			VertexBufferLayout layout =
			{
				{ ShaderDataType::Float3,	"POSITION" },
				{ ShaderDataType::UInt,		"TEXINDEX" },
				{ ShaderDataType::Float4,	"TINT" },
				{ ShaderDataType::Float2,	"UV" },
				{ ShaderDataType::UInt,		"ID" }
			};

			FramebufferSpecification specs;
			specs.existingAttachments.push_back(myUberPipeline->GetSpecification().targetFramebuffer->GetTarget());
			specs.existingAttachments.push_back(myGBufferPipeline->GetSpecification().targetFramebuffer->GetTarget(5));
			specs.existingDepthAttachment = myGBufferPipeline->GetSpecification().targetFramebuffer->GetDepthAttachment();
			specs.clearColorOnLoad = false;
			specs.clearDepthOnLoad = false;

			PipelineSpecification pipelineSpecs("Text");
			pipelineSpecs.targetFramebuffer = Framebuffer::Create(specs);
			pipelineSpecs.shader = Renderer::GetShaderLibrary()->Get("Text");
			pipelineSpecs.vertexLayouts.push_back(layout);
			//pipelineSpecs.blendMode = BlendMode::Alpha;
			pipelineSpecs.rasterizerState = RasterizerState::CullNone;
			myTextPipeline = RenderPipeline::Create(pipelineSpecs);
		}

		//Debug Render Pipeline
		{
			FramebufferSpecification specs;
			specs.debugName = "Debug Render";
			specs.attachments = { { TextureFormat::RGBA, "Debug Target" }, { TextureFormat::DEPTH32, "Depth" } };

			PipelineSpecification pipelineSpecs("Debug Render");
			pipelineSpecs.targetFramebuffer = Framebuffer::Create(specs);
			pipelineSpecs.shader = Renderer::GetShaderLibrary()->Get("DebugRender");
			pipelineSpecs.vertexLayouts.push_back(vertexLayout);
			pipelineSpecs.vertexLayouts.push_back(instanceLayout);
			myDebugRenderPipeline = RenderPipeline::Create(pipelineSpecs);
		}

		//External Compositing Framebuffer
		{
			FramebufferSpecification specs;
			specs.existingAttachments.push_back(myUberPipeline->GetSpecification().targetFramebuffer->GetTarget());
			specs.existingAttachments.push_back(myGBufferPipeline->GetSpecification().targetFramebuffer->GetTarget(5));
			specs.existingDepthAttachment = myGBufferPipeline->GetSpecification().targetFramebuffer->GetDepthAttachment();
			specs.clearColorOnLoad = false;
			specs.clearDepthOnLoad = false;

			myExternalCompositingFramebuffer = Framebuffer::Create(specs);
		}
	}

	void SceneRenderer::Shutdown()
	{
	}

	void SceneRenderer::PreRender()
	{
	}

	void SceneRenderer::UpdateStatistics()
	{
		EPOCH_PROFILE_FUNC();

		myStats = Stats();

		struct SM
		{
			uint64_t mesh;
			uint32_t index;

			bool operator<(const SM& other) const
			{
				if (mesh < other.mesh) return true;
				if (mesh > other.mesh) return false;
				return index > other.index;
			}
		};

		std::unordered_set<AssetHandle> meshes;
		std::set<SM> submeshes;

		//Instanced
		for (auto [k, dc] : myDrawList)
		{
			const auto& submesh = dc.mesh->GetSubmeshes()[dc.submeshIndex];
			myStats.instances += dc.instanceCount;
			myStats.drawCalls += CU::Math::CeilToUInt((float)dc.instanceCount / MaxInstanceCount);
			myStats.vertices += submesh.vertexCount * dc.instanceCount;
			myStats.indices += submesh.indexCount * dc.instanceCount;
			myStats.triangles += (uint32_t)dc.mesh->GetTriangleCache(dc.submeshIndex).size() * dc.instanceCount;

			meshes.insert(dc.mesh->GetHandle());
			submeshes.insert({ dc.mesh->GetHandle(), dc.submeshIndex });
		}

		////Animated
		//for (const auto& dc : myAnimatedDrawList)
		//{
		//	myStats.drawCalls += 1;
		//	myStats.vertices += dc.mesh->GetVertexBuffer()->GetCount();
		//	myStats.indices += dc.mesh->GetIndexBuffer()->GetCount();
		//	myStats.triangles += dc.mesh->GetTriangleCount();
		//}

		myStats.meshes = (uint32_t)meshes.size();
		myStats.submeshes = (uint32_t)submeshes.size();

		//Sprites
		{
			myStats.drawCalls += CU::Math::CeilToUInt((float)myQuadCount / MaxQuads);
			myStats.batched += myQuadCount;
			myStats.vertices += myQuadCount * 4;
			myStats.indices += myQuadCount * 6;
			myStats.triangles += myQuadCount * 2;
		}

		//Text
		{
			myStats.drawCalls += CU::Math::CeilToUInt((float)myTextQuadCount / MaxQuads);
			myStats.batched += myTextQuadCount;
			myStats.vertices += myTextQuadCount * 4;
			myStats.indices += myTextQuadCount * 6;
			myStats.triangles += myTextQuadCount * 2;
		}

		if (myStats.drawCalls > myStats.instances + myStats.batched)
		{
			myStats.savedDraws = 0;
		}
		else
		{
			myStats.savedDraws = myStats.instances + myStats.batched - myStats.drawCalls;
		}
	}

	void SceneRenderer::SetViewportSize(unsigned aWidth, unsigned aHeight)
	{
		if (myViewportWidth != aWidth || myViewportHeight != aHeight)
		{
			myViewportWidth = aWidth;
			myViewportHeight = aHeight;
			myInvViewportWidth = 1.0f / (float)aWidth;
			myInvViewportHeight = 1.0f / (float)aHeight;
			myNeedsResize = true;
		}
	}

	void SceneRenderer::SetScene(std::shared_ptr<Scene> aScene)
	{
		EPOCH_ASSERT(!myActive, "Can't change scenes while rendering!");
		myScene = aScene;
	}

	void SceneRenderer::BeginScene(const SceneRendererCamera& aCamera)
	{
		EPOCH_PROFILE_FUNC();

		EPOCH_ASSERT(myScene, "No scene set to render!");
		EPOCH_ASSERT(!myActive, "Can't call begin scene while rendering!");
		myActive = true;

		mySceneData.sceneCamera = aCamera;
		mySceneData.lightEnvironment = myScene->myLightEnvironment;
		mySceneData.postProcessingData = myScene->myPostProcessingData;

		if (myNeedsResize)
		{
			myNeedsResize = false;

			myGBufferPipeline->GetSpecification().targetFramebuffer->Resize(myViewportWidth, myViewportHeight);
			myEnvironmentalLightPipeline->GetSpecification().targetFramebuffer->Resize(myViewportWidth, myViewportHeight);
			myPointLightPipeline->GetSpecification().targetFramebuffer->Resize(myViewportWidth, myViewportHeight);
			mySpotlightPipeline->GetSpecification().targetFramebuffer->Resize(myViewportWidth, myViewportHeight);
			mySpritePipeline->GetSpecification().targetFramebuffer->Resize(myViewportWidth, myViewportHeight);
			myTextPipeline->GetSpecification().targetFramebuffer->Resize(myViewportWidth, myViewportHeight);
			myDebugRenderPipeline->GetSpecification().targetFramebuffer->Resize(myViewportWidth, myViewportHeight);
			myUberPipeline->GetSpecification().targetFramebuffer->Resize(myViewportWidth, myViewportHeight);

			myExternalCompositingFramebuffer->Resize(myViewportWidth, myViewportHeight);
		}

		CameraBuffer camBuffer;
		camBuffer.viewProjection = aCamera.viewMatrix * aCamera.camera.GetProjectionMatrix();
		camBuffer.pos = aCamera.position;
		camBuffer.nearPlane = aCamera.nearPlane;
		camBuffer.farPlane = aCamera.farPlane;
		camBuffer.fov = aCamera.fov;
		camBuffer.viewportSize = { (float)myViewportWidth, (float)myViewportHeight };
		myCameraBuffer->SetData(&camBuffer);
		myCameraBuffer->Bind(PIPELINE_STAGE_VERTEX_SHADER | PIPELINE_STAGE_PIXEL_SHADER, 0);
	}

	void SceneRenderer::EndScene()
	{
		EPOCH_PROFILE_FUNC();

		EPOCH_ASSERT(myActive, "Can't call end scene if not rendering!");
		myActive = false;

		if (myDrawMode == DrawMode::Shaded)
		{
			//GBuffer
			{
				Renderer::SetRenderPipeline(myGBufferPipeline);

				for (auto [k, dc] : myDrawList)
				{
					SetMaterial(dc.material);

					auto& transformStorage = myMeshTransformMap[k];
					for (uint32_t i = 0; i < dc.instanceCount; i += MaxInstanceCount)
					{
						uint32_t instanceCount = CU::Math::Min(dc.instanceCount - i, MaxInstanceCount);

						myInstanceTransformBuffer->SetData(transformStorage.data(), instanceCount, i * sizeof(MeshInstanceData));
						Renderer::RenderInstancedMesh(dc.mesh, dc.submeshIndex, myInstanceTransformBuffer, instanceCount);
					}
				}

				Renderer::RemoveRenderPipeline(myGBufferPipeline);
			}

			//Lights
			{
				//Update & set light data and brdf
				{
					LightBuffer lightBuffer;

					lightBuffer.direction = mySceneData.lightEnvironment.directionalLight.direction;
					lightBuffer.color = mySceneData.lightEnvironment.directionalLight.color;
					lightBuffer.intensity = mySceneData.lightEnvironment.directionalLight.intensity;
					lightBuffer.environmentIntensity = mySceneData.lightEnvironment.environmentIntensity;

					myLightBuffer->SetData(&lightBuffer);
					myLightBuffer->Bind(PIPELINE_STAGE_PIXEL_SHADER, 2);

					auto env = mySceneData.lightEnvironment.environment.lock();

					if (env)
					{
						std::vector<ID3D11ShaderResourceView*> SRVs(2);

						auto textureCube = env->GetRadianceMap();
						auto dxTextureCube = std::dynamic_pointer_cast<DX11TextureCube>(textureCube);
						SRVs[0] = dxTextureCube->GetSRV().Get();

						auto texture = Renderer::GetBRDFLut();
						auto dxTexture = std::dynamic_pointer_cast<DX11Texture2D>(texture);
						SRVs[1] = dxTexture->GetSRV().Get();

						RHI::GetContext()->PSSetShaderResources(10, 2, SRVs.data());
					}
					else
					{
						std::vector<ID3D11ShaderResourceView*> SRVs(1);

						auto texture = Renderer::GetBRDFLut();
						auto dxTexture = std::dynamic_pointer_cast<DX11Texture2D>(texture);
						SRVs[0] = dxTexture->GetSRV().Get();

						RHI::GetContext()->PSSetShaderResources(11, 1, SRVs.data());
					}
				}

				//Set GBuffer as resource
				{
					auto gBuffer = myGBufferPipeline->GetSpecification().targetFramebuffer;

					std::vector<ID3D11ShaderResourceView*> SRVs(gBuffer->GetColorAttachmentCount());

					for (size_t i = 0; i < gBuffer->GetColorAttachmentCount(); i++)
					{
						auto texture = gBuffer->GetTarget((uint32_t)i);
						auto dxTexture = std::dynamic_pointer_cast<DX11Texture2D>(texture);
						SRVs[i] = dxTexture->GetSRV().Get();
					}

					RHI::GetContext()->PSSetShaderResources(0, (UINT)gBuffer->GetColorAttachmentCount(), SRVs.data());
				}

				Renderer::SetRenderPipeline(myEnvironmentalLightPipeline);
				Renderer::RenderQuad();
				Renderer::RemoveRenderPipeline(myEnvironmentalLightPipeline);

				Renderer::SetRenderPipeline(myPointLightPipeline);
				for (PointLight& pointLight : mySceneData.lightEnvironment.pointLights)
				{
					myPointLightBuffer->SetData(&pointLight);
					myPointLightBuffer->Bind(PIPELINE_STAGE_PIXEL_SHADER, 2);

					Renderer::RenderQuad();
				}
				Renderer::RemoveRenderPipeline(myPointLightPipeline);

				Renderer::SetRenderPipeline(mySpotlightPipeline);
				for (Spotlight& spotlight : mySceneData.lightEnvironment.spotlights)
				{
					//Set cookie
					{
						std::vector<ID3D11ShaderResourceView*> SRVs(1);
						auto cookie = AssetManager::GetAsset<Texture2D>(spotlight.cookie);
						if (cookie)
						{
							auto dxTexture = std::dynamic_pointer_cast<DX11Texture2D>(cookie);
							SRVs[0] = dxTexture->GetSRV().Get();
						}
						else
						{
							auto dxTexture = std::dynamic_pointer_cast<DX11Texture2D>(Renderer::GetWhiteTexture());
							SRVs[0] = dxTexture->GetSRV().Get();
						}
						RHI::GetContext()->PSSetShaderResources(5, 1, SRVs.data());
					}

					mySpotlightBuffer->SetData(&spotlight);
					mySpotlightBuffer->Bind(PIPELINE_STAGE_PIXEL_SHADER, 2);

					Renderer::RenderQuad();

					//Remove cookie
					{
						std::vector<ID3D11ShaderResourceView*> SRVs(1);
						RHI::GetContext()->PSSetShaderResources(5, 1, SRVs.data());
					}
				}

				Renderer::RemoveRenderPipeline(mySpotlightPipeline);

				//Remove GBuffer as resource
				{
					auto gBuffer = myGBufferPipeline->GetSpecification().targetFramebuffer;
					std::vector<ID3D11ShaderResourceView*> emptySRVs(gBuffer->GetColorAttachmentCount());
					RHI::GetContext()->PSSetShaderResources(0, (UINT)gBuffer->GetColorAttachmentCount(), emptySRVs.data());
				}

				//Remove light data and brdf
				{
					auto env = mySceneData.lightEnvironment.environment.lock();
					if (env)
					{
						std::vector<ID3D11ShaderResourceView*> emptySRVs(2);
						RHI::GetContext()->PSSetShaderResources(10, 2, emptySRVs.data());
					}
					else
					{
						std::vector<ID3D11ShaderResourceView*> emptySRVs(1);
						RHI::GetContext()->PSSetShaderResources(11, 1, emptySRVs.data());
					}
				}
			}

			//Uber
			{
				{
					std::vector<ID3D11ShaderResourceView*> SRVs(3);
			
					auto texture = myEnvironmentalLightPipeline->GetSpecification().targetFramebuffer->GetTarget();
					auto dxTexture = std::dynamic_pointer_cast<DX11Texture2D>(texture);
					SRVs[0] = dxTexture->GetSRV().Get();
					
					texture = myGBufferPipeline->GetSpecification().targetFramebuffer->GetTarget(4);
					dxTexture = std::dynamic_pointer_cast<DX11Texture2D>(texture);
					SRVs[1] = dxTexture->GetSRV().Get();

					auto lut = AssetManager::GetAsset<Texture>(mySceneData.postProcessingData.colorGradingLUT);
					if (myColorGradingEnabled && lut)
					{
						dxTexture = std::dynamic_pointer_cast<DX11Texture2D>(lut);
						SRVs[2] = dxTexture->GetSRV().Get();
					}
						else
					{
						dxTexture = std::dynamic_pointer_cast<DX11Texture2D>(Renderer::GetDefaultColorGradingLut());
						SRVs[2] = dxTexture->GetSRV().Get();
					};

					RHI::GetContext()->PSSetShaderResources(0, 3, SRVs.data());
				}
				
				myPostProcessingBuffer->SetData(&mySceneData.postProcessingData.bufferData);
				myPostProcessingBuffer->Bind(PIPELINE_STAGE_PIXEL_SHADER, 1);
			
				Renderer::SetRenderPipeline(myUberPipeline);
				Renderer::RenderQuad();
				Renderer::RemoveRenderPipeline(myUberPipeline);
			
				
				{
					std::vector<ID3D11ShaderResourceView*> emptySRVs(3);
					RHI::GetContext()->PSSetShaderResources(0, 3, emptySRVs.data());
				}
			}

			//Sprites - TODO: Fix order independent transparency https://interplayoflight.wordpress.com/2022/06/25/order-independent-transparency-part-1/
			{
				Renderer::SetRenderPipeline(mySpritePipeline);

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

				Renderer::RemoveRenderPipeline(mySpritePipeline);
			}

			//Text
			{
				Renderer::SetRenderPipeline(myTextPipeline);

				uint32_t textureCount = 0;
				std::vector<ID3D11ShaderResourceView*> SRVs;
				SRVs.reserve(MaxTextureSlots);
				for (size_t i = 0; i < MaxTextureSlots; i++)
				{
					if (myFontAtlasesSlots[i])
					{
						SRVs.push_back((ID3D11ShaderResourceView*)myFontAtlasesSlots[i]->GetView());
						++textureCount;
					}
					else
					{
						break;
					}
				}

				RHI::GetContext()->PSSetShaderResources(0, textureCount, SRVs.data());

				for (uint32_t i = 0; i < myTextQuadCount; i += MaxQuads)
				{
					uint32_t count = CU::Math::Min(myTextQuadCount - i, MaxQuads);

					myTextVertexBuffer->SetData(myTextVertices.data(), count * 4, i * 4 * sizeof(TextVertex));
					Renderer::RenderGeometry(myTextVertexBuffer, myTextIndexBuffer, count * 6);
				}

				std::vector<ID3D11ShaderResourceView*> emptySRVs(textureCount);
				RHI::GetContext()->PSSetShaderResources(0, textureCount, emptySRVs.data());

				Renderer::RemoveRenderPipeline(myTextPipeline);
			}
		}
		else
		{
			myDebugDrawModeBuffer->SetData(&myDrawMode, 4);
			myDebugDrawModeBuffer->Bind(PIPELINE_STAGE_PIXEL_SHADER, 2);
			
			Renderer::SetRenderPipeline(myDebugRenderPipeline);

			for (auto [k, dc] : myDrawList)
			{
				SetMaterial(dc.material);

				auto& transformStorage = myMeshTransformMap[k];
				for (uint32_t i = 0; i < dc.instanceCount; i += MaxInstanceCount)
				{
					uint32_t instanceCount = CU::Math::Min(dc.instanceCount - i, MaxInstanceCount);

					myInstanceTransformBuffer->SetData(transformStorage.data(), instanceCount, i * sizeof(MeshInstanceData));
					Renderer::RenderInstancedMesh(dc.mesh, dc.submeshIndex, myInstanceTransformBuffer, instanceCount);
				}
			}

			Renderer::RemoveRenderPipeline(myDebugRenderPipeline);
		}

		UpdateStatistics();

		myDrawList.clear();
		myMeshTransformMap.clear();
		myAnimatedDrawList.clear();

		myQuadVertices.clear();
		myQuadCount = 0;

		myTextVertices.clear();
		myTextQuadCount = 0;

		for (size_t i = 1; i < MaxTextureSlots; i++)
		{
			myTextureSlots[i] = nullptr;
			myFontAtlasesSlots[i] = nullptr;
		}
	}

	void SceneRenderer::SubmitMesh(std::shared_ptr<Mesh> aMesh, std::shared_ptr<MaterialTable> aMaterialTable, const CU::Matrix4x4f& aTransform, uint32_t aEntityID)
	{
		const auto& submeshData = aMesh->GetSubmeshes();

		for (uint32_t submeshIndex = 0; submeshIndex < (uint32_t)submeshData.size(); submeshIndex++)
		{
			const Submesh& submesh = submeshData[submeshIndex];
			const CU::Matrix4x4f submeshTransform = aTransform * submesh.transform;
			uint32_t materialIndex = submesh.materialIndex;

			//for (size_t i = submesh.baseVertex; i < submesh.baseVertex + submesh.vertexCount; i++)
			//{
			//	const auto& v = aMesh->myVertices[i];
			//	const auto pos = CU::Vector3f(submeshTransform * CU::Vector4f(v.position, 1.0f));
			//	const auto dir = (CU::Matrix3x3(submeshTransform) * v.normal).GetNormalized();
			//	myDebugRenderer->DrawLine(pos, pos + dir * 15.0f, CU::Color::Magenta);
			//}

			AssetHandle materialHandle = aMaterialTable->GetMaterial(materialIndex);
			std::shared_ptr<Material> material;
			if (materialHandle != 0)
			{
				material = AssetManager::GetAsset<Material>(materialHandle);
			}

			if (materialHandle == 0 || !material)
			{
				materialHandle = Hash::GenerateFNVHash("Default-Material");
				material = AssetManager::GetAsset<Material>(materialHandle);
			}

			EPOCH_ASSERT(material, "No material found for rendering!");

			MeshKey meshKey = { aMesh->GetHandle(), materialHandle, submeshIndex, false };

			auto& transformStorage = myMeshTransformMap[meshKey].emplace_back();
			transformStorage.row[0] = { submeshTransform(1, 1), submeshTransform(1, 2), submeshTransform(1, 3), submeshTransform(4, 1) };
			transformStorage.row[1] = { submeshTransform(2, 1), submeshTransform(2, 2), submeshTransform(2, 3), submeshTransform(4, 2) };
			transformStorage.row[2] = { submeshTransform(3, 1), submeshTransform(3, 2), submeshTransform(3, 3), submeshTransform(4, 3) };
			transformStorage.id = aEntityID;

			auto& drawCall = myDrawList[meshKey];
			drawCall.mesh = aMesh;
			drawCall.submeshIndex = submeshIndex;
			drawCall.material = material;
			drawCall.instanceCount++;
		}
	}

	void SceneRenderer::SubmitAnimatedMesh(std::shared_ptr<Mesh> aMesh, const CU::Matrix4x4f& aTransform, const std::vector<CU::Matrix4x4f>& aBoneTransforms)
	{
		auto& drawCall = myAnimatedDrawList.emplace_back();
		drawCall.mesh = aMesh;
		drawCall.transform = aTransform;
		drawCall.boneTransforms = aBoneTransforms;
	}

	void SceneRenderer::SubmitQuad(const CU::Matrix4x4f aTransform, const CU::Color& aColor, uint32_t aEntityID)
	{
		SubmitQuad(aTransform, nullptr, aColor, aEntityID);
	}

	static CU::Vector2f FlipUVCoord(const CU::Vector2f& aCoord, bool aFlipX, bool aFlipY)
	{
		return { CU::Math::Abs(aCoord.x - (float)aFlipX), CU::Math::Abs(aCoord.y - (float)aFlipY) };
	}

	void SceneRenderer::SubmitQuad(const CU::Matrix4x4f aTransform, std::shared_ptr<Texture2D> aTexture, const CU::Color& aTint, bool aFlipX, bool aFlipY, uint32_t aEntityID)
	{
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

		CU::Vector4f sizeMultiplier = CU::Vector4f::One;
		if (aTexture)
		{
			if ((float)aTexture->GetWidth() < (float)aTexture->GetHeight())
			{
				sizeMultiplier.y = (float)aTexture->GetHeight() / (float)aTexture->GetWidth();
			}
			else
			{
				sizeMultiplier.x = (float)aTexture->GetWidth() / (float)aTexture->GetHeight();
			}
		}

		for (size_t i = 0; i < 4; i++)
		{
			QuadVertex& vertex = myQuadVertices.emplace_back();
			vertex.position = aTransform * (myQuadVertexPositions[i] * sizeMultiplier);
			vertex.uv = FlipUVCoord(myQuadUVCoords[i], aFlipX, aFlipY);
			vertex.tint = aTint.GetVector4();
			vertex.texIndex = textureIndex;
			vertex.entityID = aEntityID;
		}
		++myQuadCount;
	}

	static bool NextLine(int aIndex, const std::vector<int>& aLines)
	{
		for (int line : aLines)
		{
			if (line == aIndex)
			{
				return true;
			}
		}
		return false;
	}

	// warning C4996: 'std::codecvt_utf8<char32_t,1114111,(std::codecvt_mode)0>': warning STL4017: std::wbuffer_convert, std::wstring_convert, and the <codecvt> header
	// (containing std::codecvt_mode, std::codecvt_utf8, std::codecvt_utf16, and std::codecvt_utf8_utf16) are deprecated in C++17. (The std::codecvt class template is NOT deprecated.)
	// The C++ Standard doesn't provide equivalent non-deprecated functionality; consider using MultiByteToWideChar() and WideCharToMultiByte() from <Windows.h> instead.
	// You can define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING or _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS to acknowledge that you have received this warning.
#pragma warning(disable : 4996)

	// From https://stackoverflow.com/questions/31302506/stdu32string-conversion-to-from-stdstring-and-stdu16string
	static std::u32string To_UTF32(const std::string& s)
	{
		std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
		return conv.from_bytes(s);
	}

#pragma warning(default : 4996)

	enum class VerticalAlignment { Upper, Middle, Lower };
	enum class HorizontalAlignment { Left, Center, Right };

	static void VerticallyOffsetBasedOnAlignment(VerticalAlignment aAlligment, double& aVal, double aHeight, double aAddHeight)
	{
		switch (aAlligment)
		{
		case VerticalAlignment::Upper:
		{
			aVal -= aHeight - aAddHeight;
			break;
		}
		case VerticalAlignment::Middle:
		{
			aVal -= aHeight * 0.5f;
			break;
		}
		case VerticalAlignment::Lower:
		{
			aVal -= aAddHeight;
			break;
		}
		}
	}

	static void HorizontalyOffsetBasedOnAlignment(HorizontalAlignment aAlligment, double& aVal, double aWidth, double aMaxWidth)
	{
		switch (aAlligment)
		{
		case HorizontalAlignment::Left:
		{
			aVal -= aMaxWidth;
			break;
		}
		case HorizontalAlignment::Center:
		{
			aVal -= aWidth * 0.5f;
			break;
		}
		case HorizontalAlignment::Right:
		{
			if (aWidth != aMaxWidth)
			{
				aVal += (aMaxWidth - aWidth * 0.5f) * 0.5f;
			}
			break;
		}
		}
	}

	void SceneRenderer::SubmitText(const std::string& aString, const std::shared_ptr<Font>& aFont, const CU::Matrix4x4f& aTransform, const TextSettings& aSettings, uint32_t aEntityID)
	{
		if (aString.empty()) return;

		EPOCH_PROFILE_FUNC();

		std::shared_ptr<Texture2D> fontAtlas = aFont->GetFontAtlas();
		EPOCH_ASSERT(fontAtlas, "Font didn't have a valid font atlas!");

		uint32_t textureIndex = 0;
		if (aFont)
		{
			for (size_t i = 0; i < MaxTextureSlots; i++)
			{
				if (!myFontAtlasesSlots[i])
				{
					myFontAtlasesSlots[i] = aFont->GetFontAtlas();
					textureIndex = (uint32_t)i;
					break;
				}
				else if (myFontAtlasesSlots[i]->GetHandle() == aFont->GetHandle())
				{
					textureIndex = (uint32_t)i;
					break;
				}
			}
		}

		auto& fontGeometry = aFont->GetMSDFData()->fontGeometry;
		const auto& metrics = fontGeometry.getMetrics();

		std::u32string utf32string = To_UTF32(aString);

		double totalHeight;
		double maxWidth = 0.0f;
		std::vector<int> nextLines;
		std::vector<double> lineWidths;
		{
			double x = 0.0;
			double fsScale = 1 / (metrics.ascenderY - metrics.descenderY);
			double y = -fsScale * metrics.ascenderY;
			const double startHeight = y;
			int lastSpace = -1;
			for (int i = 0; i < utf32string.size(); i++)
			{
				char32_t character = utf32string[i];
				if (character == '\n')
				{
					if (x > maxWidth) maxWidth = x;
					lineWidths.push_back(x);
					x = 0;
					y -= fsScale * metrics.lineHeight + aSettings.lineHeightOffset;
					continue;
				}
				if (character == '\r')
				{
					continue;
				}

				const bool isTab = character == '\t';
				if (isTab) character = ' ';

				auto glyph = fontGeometry.getGlyph(character);
				//if (!glyph)
				//{
				//	glyph = fontGeometry.getGlyph('?');}
				//}
				if (!glyph)
				{
					continue;
				}

				if (character != ' ')
				{
					// Calculate geo
					double pl, pb, pr, pt;
					glyph->getQuadPlaneBounds(pl, pb, pr, pt);
					CU::Vector2f quadMin((float)pl, (float)pb);
					CU::Vector2f quadMax((float)pr, (float)pt);

					quadMin *= (float)fsScale;
					quadMax *= (float)fsScale;
					quadMin += CU::Vector2f((float)x, (float)y);
					quadMax += CU::Vector2f((float)x, (float)y);

					if (quadMax.x > aSettings.maxWidth && lastSpace != -1)
					{
						i = lastSpace;
						lineWidths.push_back(x);
						nextLines.emplace_back(lastSpace);
						lastSpace = -1;
						x = 0;
						y -= fsScale * metrics.lineHeight + aSettings.lineHeightOffset;
					}
				}
				else
				{
					lastSpace = i;
				}

				double advance = glyph->getAdvance();
				fontGeometry.getAdvance(advance, character, utf32string[i + 1]);
				x += (fsScale * advance + aSettings.letterSpacing) * (isTab ? 4 : 1);
			}
			if (x > maxWidth) maxWidth = x;
			lineWidths.push_back(x);
			totalHeight = startHeight - y;
		}

		//VerticalAlignment verticalAlignment = (VerticalAlignment)((int)aAlignment / 3);
		//HorizontalAlignment horizontalAlignment = (HorizontalAlignment)((int)aAlignment % 3);

		{
			unsigned line = 0;
			double x = 0.0f;
			if (aSettings.centered) HorizontalyOffsetBasedOnAlignment(HorizontalAlignment::Center, x, lineWidths[line], maxWidth);
			double fsScale = 1 / (metrics.ascenderY - metrics.descenderY);
			double y = 0.0;
			if (aSettings.centered) VerticallyOffsetBasedOnAlignment(VerticalAlignment::Middle, y, -totalHeight, (fsScale * metrics.lineHeight + aSettings.lineHeightOffset) * 0.5f);
			for (int i = 0; i < utf32string.size(); i++)
			{
				char32_t character = utf32string[i];
				if (character == '\n' || NextLine(i, nextLines))
				{
					++line;
					x = 0.0;
					if (aSettings.centered) HorizontalyOffsetBasedOnAlignment(HorizontalAlignment::Center, x, lineWidths[line], maxWidth);
					y -= fsScale * metrics.lineHeight + aSettings.lineHeightOffset;
					continue;
				}

				const bool isTab = character == '\t';
				if (isTab) character = ' ';

				auto glyph = fontGeometry.getGlyph(character);
				//if (!glyph)
				//{
				//	glyph = fontGeometry.getGlyph('?');
				//}
				if (!glyph)
				{
					continue;
				}

				double l, b, r, t;
				glyph->getQuadAtlasBounds(l, b, r, t);

				double pl, pb, pr, pt;
				glyph->getQuadPlaneBounds(pl, pb, pr, pt);

				pl *= fsScale, pb *= fsScale, pr *= fsScale, pt *= fsScale;
				pl += x, pb += y, pr += x, pt += y;


				VerticalAlignment vertAlign = aSettings.centered ? VerticalAlignment::Middle : VerticalAlignment::Upper;
				VerticallyOffsetBasedOnAlignment(vertAlign, pb, (fsScale * metrics.lineHeight + aSettings.lineHeightOffset) * 0.5f, 0);
				VerticallyOffsetBasedOnAlignment(vertAlign, pt, (fsScale * metrics.lineHeight + aSettings.lineHeightOffset) * 0.5f, 0);

				double texelWidth = 1. / fontAtlas->GetWidth();
				double texelHeight = 1. / fontAtlas->GetHeight();
				l *= texelWidth, b *= texelHeight, r *= texelWidth, t *= texelHeight;

				if (character != ' ')
				{
					{
						TextVertex& vertex = myTextVertices.emplace_back();
						vertex.position = aTransform * CU::Vector4f((float)pl * 100.0f, (float)pb * 100.0f, 0.0f, 1.0f);
						vertex.tint = aSettings.color.GetVector4();
						vertex.uv = { (float)l, (float)b };
						vertex.texIndex = textureIndex;
						vertex.entityID = aEntityID;
					}

					{
						TextVertex& vertex = myTextVertices.emplace_back();
						vertex.position = aTransform * CU::Vector4f((float)pl * 100.0f, (float)pt * 100.0f, 0.0f, 1.0f);
						vertex.tint = aSettings.color.GetVector4();
						vertex.uv = { (float)l, (float)t };
						vertex.texIndex = textureIndex;
						vertex.entityID = aEntityID;
					}

					{
						TextVertex& vertex = myTextVertices.emplace_back();
						vertex.position = aTransform * CU::Vector4f((float)pr * 100.0f, (float)pt * 100.0f, 0.0f, 1.0f);
						vertex.tint = aSettings.color.GetVector4();
						vertex.uv = { (float)r, (float)t };
						vertex.texIndex = textureIndex;
						vertex.entityID = aEntityID;
					}

					{
						TextVertex& vertex = myTextVertices.emplace_back();
						vertex.position = aTransform * CU::Vector4f((float)pr * 100.0f, (float)pb * 100.0f, 0.0f, 1.0f);
						vertex.tint = aSettings.color.GetVector4();
						vertex.uv = { (float)r, (float)b };
						vertex.texIndex = textureIndex;
						vertex.entityID = aEntityID;
					}

					++myTextQuadCount;
				}

				double advance = glyph->getAdvance();
				fontGeometry.getAdvance(advance, character, utf32string[i + 1]);
				x += (fsScale * advance + aSettings.letterSpacing) * (isTab ? 4 : 1);
			}
		}
	}

	std::shared_ptr<Texture2D> SceneRenderer::GetFinalPassTexture()
	{
		if (myDrawMode == DrawMode::Shaded)
		{
			return myUberPipeline->GetSpecification().targetFramebuffer->GetTarget();
		}
		else
		{
			return myDebugRenderPipeline->GetSpecification().targetFramebuffer->GetTarget();
		}
	}

	std::shared_ptr<Texture2D> SceneRenderer::GetEntityIDTexture()
	{
		if (myDrawMode == DrawMode::Shaded)
		{
			return myGBufferPipeline->GetSpecification().targetFramebuffer->GetTarget(5);
		}
		else
		{
			return nullptr;
		}
	}

	std::shared_ptr<Framebuffer> SceneRenderer::GetExternalCompositingFramebuffer()
	{
		return myExternalCompositingFramebuffer;
	}

	//Temp
	void SceneRenderer::SetMaterial(std::shared_ptr<Material> aMaterial)
	{
		EPOCH_PROFILE_FUNC();

		Material::Data matData;
		matData.albedoColor = aMaterial->GetAlbedoColor();
		matData.roughness = aMaterial->GetRoughness();
		matData.metalness = aMaterial->GetMetalness();
		matData.normalStrength = aMaterial->GetNormalStrength();
		matData.uvTiling = aMaterial->GetUVTiling();
		matData.emissionColor = aMaterial->GetEmissionColor();
		matData.emissionStrength = aMaterial->GetEmissionStrength();

		myMaterialBuffer->SetData(&matData);
		myMaterialBuffer->Bind(PIPELINE_STAGE_PIXEL_SHADER, 1);

		std::vector<ID3D11ShaderResourceView*> SRVs(3);

		{
			//auto texture = AssetManager::GetAssetAsync<Texture2D>(aMaterial->GetAlbedoTexture());
			auto texture = AssetManager::GetAsset<Texture2D>(aMaterial->GetAlbedoTexture()); //TODO: Make async
			if (!texture)
			{
				texture = Renderer::GetWhiteTexture();
			}
			auto dxTexture = std::dynamic_pointer_cast<DX11Texture2D>(texture);
			SRVs[0] = dxTexture->GetSRV().Get();
		}
		{
			//auto texture = AssetManager::GetAssetAsync<Texture2D>(aMaterial->GetNormalTexture());
			auto texture = AssetManager::GetAsset<Texture2D>(aMaterial->GetNormalTexture()); //TODO: Make async
			if (!texture)
			{
				texture = Renderer::GetFlatNormalTexture();
			}
			auto dxTexture = std::dynamic_pointer_cast<DX11Texture2D>(texture);
			SRVs[1] = dxTexture->GetSRV().Get();
		}
		{
			//auto texture = AssetManager::GetAssetAsync<Texture2D>(aMaterial->GetMaterialTexture());
			auto texture = AssetManager::GetAsset<Texture2D>(aMaterial->GetMaterialTexture()); //TODO: Make async
			if (!texture)
			{
				texture = Renderer::GetDefaultMaterialTexture();
			}
			auto dxTexture = std::dynamic_pointer_cast<DX11Texture2D>(texture);
			SRVs[2] = dxTexture->GetSRV().Get();
		}
		RHI::GetContext()->PSSetShaderResources(0, 3, SRVs.data());
	}
}
