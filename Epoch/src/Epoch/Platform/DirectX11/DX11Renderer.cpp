#include "epch.h"
#include "DX11Renderer.h"
#include "Epoch/Core/GraphicsEngine.h" //TODO: Remove TEMP
#include "Epoch/Rendering/Renderer.h"
#include "Epoch/Platform/DirectX11/DX11RenderPipeline.h"
#include "Epoch/Platform/DirectX11/DX11ComputePipeline.h"
#include "Epoch/Platform/DirectX11/DX11Framebuffer.h"
#include "Epoch/Platform/DirectX11/DX11Shader.h"
#include "Epoch/Platform/DirectX11/DX11VertexBuffer.h"
#include "Epoch/Platform/DirectX11/DX11IndexBuffer.h"
#include "Epoch/Platform/DirectX11/DX11Texture.h"
#include "Epoch/Platform/DirectX11/DX11ConstantBuffer.h"
#include "Epoch/Rendering/Mesh.h"

namespace Epoch
{
	void DX11Renderer::Init()
	{
		//TODO: Remove, temp af
		
		{
			D3D11_SAMPLER_DESC samplerDescription = {};
			samplerDescription.Filter = D3D11_FILTER_ANISOTROPIC;
			//samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDescription.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
			samplerDescription.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			samplerDescription.BorderColor[0] = 0;
			samplerDescription.BorderColor[1] = 0;
			samplerDescription.BorderColor[2] = 0;
			samplerDescription.BorderColor[3] = 0;
			samplerDescription.MinLOD = 0.0f;
			samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;

			RHI::GetDevice()->CreateSamplerState(&samplerDescription, wrapSamplerState.GetAddressOf());
			RHI::GetContext()->PSSetSamplers(0, 1, wrapSamplerState.GetAddressOf());
			RHI::GetContext()->CSSetSamplers(0, 1, wrapSamplerState.GetAddressOf());
		}
		
		{
			D3D11_SAMPLER_DESC samplerDescription = {};
			samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDescription.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			samplerDescription.BorderColor[0] = 0;
			samplerDescription.BorderColor[1] = 0;
			samplerDescription.BorderColor[2] = 0;
			samplerDescription.BorderColor[3] = 0;
			samplerDescription.MinLOD = 0.0f;
			samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;

			RHI::GetDevice()->CreateSamplerState(&samplerDescription, clampSamplerState.GetAddressOf());
			RHI::GetContext()->PSSetSamplers(1, 1, clampSamplerState.GetAddressOf());
			RHI::GetContext()->CSSetSamplers(1, 1, wrapSamplerState.GetAddressOf());
		}

		{
			D3D11_SAMPLER_DESC samplerDescription = {};
			samplerDescription.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDescription.ComparisonFunc = D3D11_COMPARISON_NEVER;

			RHI::GetDevice()->CreateSamplerState(&samplerDescription, brdfLUTSamplerState.GetAddressOf());
			RHI::GetContext()->PSSetSamplers(3, 1, brdfLUTSamplerState.GetAddressOf());
			RHI::GetContext()->CSSetSamplers(3, 1, brdfLUTSamplerState.GetAddressOf());
		}
	}

	void DX11Renderer::Shutdown()
	{
	}

	void DX11Renderer::BeginFrame()
	{
	}

	void DX11Renderer::EndFrame()
	{
	}

	void DX11Renderer::SetComputePipeline(std::shared_ptr<ComputePipeline> aComputePipeline)
	{
		EPOCH_PROFILE_FUNC();

		EPOCH_ASSERT(aComputePipeline, "Invalid render pipeline!");

		auto dxShader = std::dynamic_pointer_cast<DX11Shader>(aComputePipeline->GetSpecification().shader);

		if (dxShader->HasShader(ShaderStage::Compute))
		{
			ComPtr<ID3D11ComputeShader> cs;
			dxShader->GetShader(ShaderStage::Compute).As(&cs);
			RHI::GetContext()->CSSetShader(cs.Get(), nullptr, 0);
		}
	}

	void DX11Renderer::RemoveComputePipeline(std::shared_ptr<ComputePipeline> aComputePipeline)
	{
		EPOCH_PROFILE_FUNC();

		EPOCH_ASSERT(aComputePipeline, "Invalid render pipeline!");
	}

	void DX11Renderer::DispatchCompute(const CU::Vector3ui& aWorkGroups)
	{
		EPOCH_PROFILE_FUNC();

		RHI::GetContext()->Dispatch(aWorkGroups.x, aWorkGroups.y, aWorkGroups.z);
	}

	void DX11Renderer::SetRenderPipeline(std::shared_ptr<RenderPipeline> aRenderPipeline)
	{
		EPOCH_PROFILE_FUNC();

		EPOCH_ASSERT(aRenderPipeline, "Invalid render pipeline!");

		std::shared_ptr<DX11RenderPipeline> renderPipeline = std::dynamic_pointer_cast<DX11RenderPipeline>(aRenderPipeline);

		RHI::GetContext()->IASetInputLayout(renderPipeline->GetInputLayout().Get());
		RHI::GetContext()->IASetPrimitiveTopology(ToDX11Topology(renderPipeline->GetSpecification().primitiveTopology));

		RHI::GetContext()->RSSetState(renderPipeline->GetRasterizerState().Get());

		const std::array<float, 4>& blendFactor = { 0, 0, 0, 0 };
		RHI::GetContext()->OMSetBlendState(renderPipeline->GetBlendState().Get(), blendFactor.data(), 0xffffffff);
		
		RHI::GetContext()->OMSetDepthStencilState(renderPipeline->GetDepthStencilState().Get(), 0);
		
		auto dxShader = std::dynamic_pointer_cast<DX11Shader>(aRenderPipeline->GetSpecification().shader);

		if (dxShader->HasShader(ShaderStage::Vertex))
		{
			ComPtr<ID3D11VertexShader> vs;
			dxShader->GetShader(ShaderStage::Vertex).As(&vs);
			RHI::GetContext()->VSSetShader(vs.Get(), nullptr, 0);
		}

		if (dxShader->HasShader(ShaderStage::Pixel))
		{
			ComPtr<ID3D11PixelShader> ps;
			dxShader->GetShader(ShaderStage::Pixel).As(&ps);
			RHI::GetContext()->PSSetShader(ps.Get(), nullptr, 0);
		}

		std::shared_ptr<Framebuffer> frameBuffer = aRenderPipeline->GetSpecification().targetFramebuffer;

		std::vector<std::shared_ptr<DX11Texture2D>> colorAttachments;
		std::shared_ptr<DX11Texture2D> depthAttachment;

		uint32_t width = 0;
		uint32_t height = 0;

		if (frameBuffer->GetSpecification().swapChainTarget)
		{
			colorAttachments.push_back(std::dynamic_pointer_cast<DX11Texture2D>(GraphicsEngine::Get().GetBackBuffer()));
			depthAttachment = std::dynamic_pointer_cast<DX11Texture2D>(GraphicsEngine::Get().GetDepthBuffer());

			width = colorAttachments[0]->GetWidth();
			height = colorAttachments[0]->GetHeight();
		}
		else
		{
			colorAttachments.reserve(frameBuffer->GetColorAttachmentCount());

			for (uint32_t i = 0; i < (uint32_t)frameBuffer->GetColorAttachmentCount(); i++)
			{
				colorAttachments.push_back(std::dynamic_pointer_cast<DX11Texture2D>(frameBuffer->GetTarget(i)));
			}

			if (frameBuffer->HasDepthAttachment())
			{
				depthAttachment = std::dynamic_pointer_cast<DX11Texture2D>(frameBuffer->GetDepthAttachment());
			}

			width = frameBuffer->GetWidth();
			height = frameBuffer->GetHeight();
		}

		if (frameBuffer->GetSpecification().clearColorOnLoad)
		{
			for (auto attachment : colorAttachments)
			{
				RHI::GetContext()->ClearRenderTargetView(attachment->GetRTV().Get(), &frameBuffer->GetSpecification().clearColor.r);
			}
		}

		if (frameBuffer->GetSpecification().clearDepthOnLoad)
		{
			if (depthAttachment)
			{
				RHI::GetContext()->ClearDepthStencilView(depthAttachment->GetDSV().Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, frameBuffer->GetSpecification().depthClearValue, 0);
			}
		}

		std::vector<ID3D11RenderTargetView*> myRTVs;
		myRTVs.reserve(colorAttachments.size());

		for (auto attachment : colorAttachments)
		{
			myRTVs.push_back(attachment->GetRTV().Get());
		}

		const UINT numRenderTargets = static_cast<UINT>(myRTVs.size());
		RHI::GetContext()->OMSetRenderTargets(numRenderTargets, myRTVs.data(), depthAttachment ? depthAttachment->GetDSV().Get() : nullptr);

		const D3D11_VIEWPORT viewport(0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f);
		RHI::GetContext()->RSSetViewports(1, &viewport);
	}

	void DX11Renderer::RemoveRenderPipeline(std::shared_ptr<RenderPipeline> aRenderPipeline)
	{
		EPOCH_PROFILE_FUNC();

		EPOCH_ASSERT(aRenderPipeline, "Invalid render pipeline!");
		
		RHI::GetContext()->IASetInputLayout(nullptr);

		auto dxShader = std::dynamic_pointer_cast<DX11Shader>(aRenderPipeline->GetSpecification().shader);

		if (dxShader->HasShader(ShaderStage::Vertex))
		{
			RHI::GetContext()->VSSetShader(nullptr, nullptr, 0);
		}

		if (dxShader->HasShader(ShaderStage::Pixel))
		{
			RHI::GetContext()->PSSetShader(nullptr, nullptr, 0);
		}

		RHI::GetContext()->OMSetRenderTargets(0, nullptr, nullptr);
	}

	void DX11Renderer::RenderInstancedMesh(std::shared_ptr<Mesh> aMesh, uint32_t aSubmeshIndex, std::shared_ptr<VertexBuffer> aTransformBuffer, uint32_t aInstanceCount)
	{
		EPOCH_PROFILE_FUNC();

		const auto& meshVertexBuffer = std::dynamic_pointer_cast<DX11VertexBuffer>(aMesh->GetVertexBuffer());
		const auto& transformVertexBuffer = std::dynamic_pointer_cast<DX11VertexBuffer>(aTransformBuffer);
		const auto& meshIndexBuffer = std::dynamic_pointer_cast<DX11IndexBuffer>(aMesh->GetIndexBuffer());

		const auto& submeshes = aMesh->GetSubmeshes();
		const auto& submesh = submeshes[aSubmeshIndex];

		std::vector<ID3D11Buffer*> buffers;
		buffers.push_back(meshVertexBuffer->GetDXBuffer().Get());
		buffers.push_back(transformVertexBuffer->GetDXBuffer().Get());
		std::vector<unsigned> vxOffsets = { 0, 0 };
		std::vector<unsigned> vxStrides = { meshVertexBuffer->GetStride(), aTransformBuffer->GetStride() };

		RHI::GetContext()->IASetVertexBuffers(0, 2, buffers.data(), vxStrides.data(), vxOffsets.data());
		RHI::GetContext()->IASetIndexBuffer(meshIndexBuffer->GetDXBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

		RHI::GetContext()->DrawIndexedInstanced(submesh.indexCount, aInstanceCount, submesh.baseIndex, submesh.baseVertex, 0);
	}

	void DX11Renderer::RenderAnimatedMesh(std::shared_ptr<Mesh> aMesh)
	{
		EPOCH_PROFILE_FUNC();

		const auto& meshVertexBuffer = std::dynamic_pointer_cast<DX11VertexBuffer>(aMesh->GetVertexBuffer());
		const auto& boneInfluenceBuffer = std::dynamic_pointer_cast<DX11VertexBuffer>(aMesh->GetBoneInfluenceBuffer());
		const auto& meshIndexBuffer = std::dynamic_pointer_cast<DX11IndexBuffer>(aMesh->GetIndexBuffer());

		std::vector<ID3D11Buffer*> buffers;
		buffers.push_back(meshVertexBuffer->GetDXBuffer().Get());
		buffers.push_back(boneInfluenceBuffer->GetDXBuffer().Get());
		std::vector<unsigned> vxOffsets = { 0, 0 };
		std::vector<unsigned> vxStrides = { meshVertexBuffer->GetStride(), boneInfluenceBuffer->GetStride() };

		RHI::GetContext()->IASetVertexBuffers(0, 2, buffers.data(), vxStrides.data(), vxOffsets.data());
		RHI::GetContext()->IASetIndexBuffer(meshIndexBuffer->GetDXBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

		RHI::GetContext()->DrawIndexed(meshIndexBuffer->GetCount(), 0, 0);
	}

	void DX11Renderer::RenderQuad()
	{
		EPOCH_PROFILE_FUNC();

		RHI::GetContext()->Draw(6, 0);
	}

	void DX11Renderer::RenderGeometry(std::shared_ptr<VertexBuffer> aVertexBuffer, std::shared_ptr<IndexBuffer> aIndexBuffer, uint32_t aIndexCount)
	{
		EPOCH_PROFILE_FUNC();

		if (aIndexCount == 0) aIndexCount = aIndexBuffer->GetCount();

		const auto& dxVertexBuffer = std::dynamic_pointer_cast<DX11VertexBuffer>(aVertexBuffer);
		const auto& dxIndexBuffer = std::dynamic_pointer_cast<DX11IndexBuffer>(aIndexBuffer);

		constexpr unsigned vxOffset = 0;
		const UINT stride = dxVertexBuffer->GetStride();
		RHI::GetContext()->IASetVertexBuffers(0, 1, dxVertexBuffer->GetDXBuffer().GetAddressOf(), &stride, &vxOffset);
		RHI::GetContext()->IASetIndexBuffer(dxIndexBuffer->GetDXBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

		RHI::GetContext()->DrawIndexed(aIndexCount, 0, 0);
	}

	std::pair<std::shared_ptr<TextureCube>, std::shared_ptr<TextureCube>> DX11Renderer::CreateEnvironmentTextures(const std::string& aFilepath)
	{
		Timer timer;

		//LOG_WARNING("'CreateEnvironmentTextures' not implemented!");
		//return std::pair<std::shared_ptr<TextureCube>, std::shared_ptr<TextureCube>>(nullptr, nullptr);
		
		std::shared_ptr<Texture2D> envEquirect = Texture2D::Create(aFilepath);
		EPOCH_ASSERT(envEquirect->GetFormat() == TextureFormat::RGBA32F, "Texture is not HDR!");
		
		const uint32_t cubemapSize = 1024;
		const uint32_t irradianceMapSize = 32;
		
		TextureSpecification cubemapSpec;
		cubemapSpec.generateMips = true;
		cubemapSpec.usage = TextureUsage::Storage;
		cubemapSpec.format = TextureFormat::RGBA32F;
		cubemapSpec.width = cubemapSize;
		cubemapSpec.height = cubemapSize;
		cubemapSpec.debugName = std::filesystem::path(aFilepath).stem().string();
		
		std::shared_ptr<TextureCube> envUnfiltered = TextureCube::Create(cubemapSpec);
		std::shared_ptr<TextureCube> envFiltered = TextureCube::Create(cubemapSpec);
		
		// Convert equirectangular to cubemap
		{
			std::shared_ptr<Shader> equirectangularConversionShader = Renderer::GetShaderLibrary()->Get("EquirectangularToCubeMap");
			auto dxShader = std::dynamic_pointer_cast<DX11Shader>(equirectangularConversionShader);

			ComPtr<ID3D11ComputeShader> cs;
			dxShader->GetShader(ShaderStage::Compute).As(&cs);
			RHI::GetContext()->CSSetShader(cs.Get(), nullptr, 0);

			std::shared_ptr<DX11TextureCube> dxTextureCube = std::dynamic_pointer_cast<DX11TextureCube>(envUnfiltered);
			std::shared_ptr<DX11Texture2D> dxEquirect = std::dynamic_pointer_cast<DX11Texture2D>(envEquirect);
			RHI::GetContext()->CSSetUnorderedAccessViews(0, 1, dxTextureCube->GetUAV().GetAddressOf(), 0);
			RHI::GetContext()->CSSetShaderResources(0, 1, dxEquirect->GetSRV().GetAddressOf());

			DispatchCompute({ cubemapSize / 32, cubemapSize / 32 , 6 });

			std::vector<ID3D11UnorderedAccessView*> emptyUAVs(1);
			std::vector<ID3D11ShaderResourceView*> emptySRVs(1);
			RHI::GetContext()->CSSetUnorderedAccessViews(0, 1, emptyUAVs.data(), 0);
			RHI::GetContext()->CSSetShaderResources(0, 1, emptySRVs.data());

			RHI::GetContext()->CSSetShader(nullptr, nullptr, 0);
			
			dxTextureCube->GenerateMips();
		}

		// Filter cubemap
		{
			std::shared_ptr<ConstantBuffer> cb = ConstantBuffer::Create(sizeof(CU::Vector4f));

			std::shared_ptr<Shader> equirectangularConversionShader = Renderer::GetShaderLibrary()->Get("EnvironmentMipFilter");
			auto dxShader = std::dynamic_pointer_cast<DX11Shader>(equirectangularConversionShader);

			ComPtr<ID3D11ComputeShader> cs;
			dxShader->GetShader(ShaderStage::Compute).As(&cs);
			RHI::GetContext()->CSSetShader(cs.Get(), nullptr, 0);

			std::shared_ptr<DX11TextureCube> dxOutput = std::dynamic_pointer_cast<DX11TextureCube>(envFiltered);
			std::shared_ptr<DX11TextureCube> dxInput = std::dynamic_pointer_cast<DX11TextureCube>(envUnfiltered);
			
			uint32_t size = cubemapSize;
			const float deltaRoughness = 1.0f / CU::Math::Max((float)envFiltered->GetMipLevelCount() - 1.0f, 1.0f);
			const uint32_t mipCount = (uint32_t)CU::Math::Floor(std::log2f((float)CU::Math::Min(cubemapSize, cubemapSize))) + 1;
			for (size_t i = 0; i < mipCount; i++)
			{
				uint32_t numGroups = CU::Math::Max(1u, size / 32u);
				float roughness = i * deltaRoughness;
				roughness = CU::Math::Max(roughness, 0.05f);

				cb->SetData(&roughness);
				cb->Bind(PIPELINE_STAGE_PIXEL_SHADER, 0);

				ComPtr<ID3D11UnorderedAccessView> mipUAV = dxOutput->GetMipUAV((uint32_t)i);

				RHI::GetContext()->CSSetUnorderedAccessViews(0, 1, mipUAV.GetAddressOf(), 0);
				RHI::GetContext()->CSSetShaderResources(0, 1, dxInput->GetSRV().GetAddressOf());

				DispatchCompute({ cubemapSize / 32, cubemapSize / 32 , 6 });

				std::vector<ID3D11UnorderedAccessView*> emptyUAVs(1);
				std::vector<ID3D11ShaderResourceView*> emptySRVs(1);
				RHI::GetContext()->CSSetUnorderedAccessViews(0, 1, emptyUAVs.data(), 0);
				RHI::GetContext()->CSSetShaderResources(0, 1, emptySRVs.data());

				size /= 2;
			}
			
			RHI::GetContext()->CSSetShader(nullptr, nullptr, 0);
		}

		LOG_DEBUG_TAG("Renderer", "Creation of environmental map took {}ms", timer.ElapsedMillis());
		return std::pair<std::shared_ptr<TextureCube>, std::shared_ptr<TextureCube>>(envFiltered, envUnfiltered);
		//return std::pair<std::shared_ptr<TextureCube>, std::shared_ptr<TextureCube>>(envUnfiltered, envFiltered);
	}
}
