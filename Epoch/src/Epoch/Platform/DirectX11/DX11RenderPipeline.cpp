#include "epch.h"
#include "DX11RenderPipeline.h"
#include "Epoch/Rendering/RHI.h" //TODO: Temp, remove
#include "DX11Shader.h"

namespace Epoch
{
	DX11RenderPipeline::DX11RenderPipeline(const PipelineSpecification& aSpec)
	{
		EPOCH_PROFILE_FUNC();

		mySpecification = aSpec;

		// Vertex input descriptor
		{
			EPOCH_PROFILE_SCOPE("DX11RenderPipeline::VertexInputDescriptor");

			uint32_t totalElementCount = 0;
			for (const auto& layout : aSpec.vertexLayouts)
			{
				totalElementCount += layout.GetElementCount();
			}

			if (totalElementCount > 0)
			{
				std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDefinition(totalElementCount);

				uint32_t location = 0;
				uint32_t inputSlot = 0;
				for (const auto& layout : aSpec.vertexLayouts)
				{
					D3D11_INPUT_CLASSIFICATION inputRate = (layout.GetInpuRate() == ShaderDataInputRate::PerVertex) ? D3D11_INPUT_PER_VERTEX_DATA : D3D11_INPUT_PER_INSTANCE_DATA;
					for (const auto& element : layout)
					{
						inputLayoutDefinition[location].SemanticName = element.name.c_str();
						inputLayoutDefinition[location].SemanticIndex = element.index;
						inputLayoutDefinition[location].Format = ShaderDataTypeToDX11Format(element.type);
						inputLayoutDefinition[location].InputSlot = inputSlot;
						inputLayoutDefinition[location].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
						inputLayoutDefinition[location].InputSlotClass = inputRate;
						inputLayoutDefinition[location].InstanceDataStepRate = (UINT)inputRate;
						location++;
					}
					inputSlot++;
				}

				auto dx11Shader = std::dynamic_pointer_cast<DX11Shader>(aSpec.shader);
				if (dx11Shader->HasShader(ShaderStage::Vertex))
				{
					auto& shaderData = dx11Shader->GetShaderData(ShaderStage::Vertex);
					RHI::GetDevice()->CreateInputLayout(inputLayoutDefinition.data(), static_cast<UINT>(inputLayoutDefinition.size()), shaderData.data(), shaderData.size(), myInputLayout.GetAddressOf());
				}
				else
				{
					LOG_ERROR("Shader '{}' specified in pipeline state '{}' doesn't have a vertex shader", aSpec.shader->GetName(), aSpec.debugName);
					EPOCH_ASSERT(false, "A vertex shader has to be provided for the creation of a render pipeline state");
				}
			}
		}

		// States
		{
			EPOCH_PROFILE_SCOPE("DX11RenderPipeline::States");

			// Rasterizer state
			{
				D3D11_RASTERIZER_DESC rasterizerDescription;
				rasterizerDescription.FrontCounterClockwise = false;
				rasterizerDescription.DepthBias = 0;
				rasterizerDescription.SlopeScaledDepthBias = 0.0f;
				rasterizerDescription.DepthBiasClamp = 0.0f;
				rasterizerDescription.DepthClipEnable = true;
				rasterizerDescription.ScissorEnable = false;
				rasterizerDescription.MultisampleEnable = false;
				rasterizerDescription.AntialiasedLineEnable = false;
				rasterizerDescription.FillMode = ToDX11FillMode(aSpec.rasterizerState);
				rasterizerDescription.CullMode = ToDX11CullMode(aSpec.rasterizerState);

				RHI::GetDevice()->CreateRasterizerState(&rasterizerDescription, myRasterizerState.GetAddressOf());
			}

			// Blend state //TODO: Move to frame buffer so that every render target can have it's own blend state
			{
				D3D11_BLEND_DESC blendDescription{};
				D3D11_RENDER_TARGET_BLEND_DESC& targetBlendDescription = blendDescription.RenderTarget[0];

				targetBlendDescription.BlendEnable = FALSE;
				targetBlendDescription.SrcBlend = D3D11_BLEND_ONE;
				targetBlendDescription.DestBlend = D3D11_BLEND_ZERO;
				targetBlendDescription.BlendOp = D3D11_BLEND_OP_ADD;
				targetBlendDescription.SrcBlendAlpha = D3D11_BLEND_ONE;
				targetBlendDescription.DestBlendAlpha = D3D11_BLEND_ZERO;
				targetBlendDescription.BlendOpAlpha = D3D11_BLEND_OP_ADD;
				targetBlendDescription.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

				switch (aSpec.blendMode)
				{
				case BlendMode::Alpha:
				{
					targetBlendDescription.BlendEnable = TRUE;
					targetBlendDescription.SrcBlend = D3D11_BLEND_SRC_ALPHA;
					targetBlendDescription.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
					targetBlendDescription.BlendOp = D3D11_BLEND_OP_ADD;
					targetBlendDescription.SrcBlendAlpha = D3D11_BLEND_DEST_ALPHA;
					targetBlendDescription.DestBlendAlpha = D3D11_BLEND_ONE;
					targetBlendDescription.BlendOpAlpha = D3D11_BLEND_OP_ADD;
					targetBlendDescription.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

					break;
				}
				case BlendMode::Additive:
				{
					targetBlendDescription.BlendEnable = TRUE;
					targetBlendDescription.SrcBlend = D3D11_BLEND_SRC_ALPHA;
					targetBlendDescription.DestBlend = D3D11_BLEND_ONE;
					targetBlendDescription.BlendOp = D3D11_BLEND_OP_ADD;
					targetBlendDescription.SrcBlendAlpha = D3D11_BLEND_ZERO;
					targetBlendDescription.DestBlendAlpha = D3D11_BLEND_ONE;
					targetBlendDescription.BlendOpAlpha = D3D11_BLEND_OP_ADD;
					targetBlendDescription.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

					break;
				}
				}

				RHI::GetDevice()->CreateBlendState(&blendDescription, myBlendState.GetAddressOf());
			}

			// Depth state
			{
				D3D11_DEPTH_STENCIL_DESC depthStencilDescription;
				depthStencilDescription.DepthEnable = aSpec.depthTest;
				depthStencilDescription.DepthFunc = ToDX11CompareOperator(aSpec.depthCompareOperator);
				depthStencilDescription.DepthWriteMask = aSpec.depthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
				depthStencilDescription.StencilEnable = false;

				RHI::GetDevice()->CreateDepthStencilState(&depthStencilDescription, myDepthState.GetAddressOf());
			}
		}
	}
}
