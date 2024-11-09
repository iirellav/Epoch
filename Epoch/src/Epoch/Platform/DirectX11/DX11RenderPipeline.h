#pragma once
#include "Epoch/Rendering/RenderPipeline.h"
#include <d3d11.h>
#include <wrl.h>

using namespace Microsoft::WRL;

namespace Epoch
{
	static DXGI_FORMAT ShaderDataTypeToDX11Format(ShaderDataType aType)
	{
		switch (aType)
		{
		case ShaderDataType::Float:		return DXGI_FORMAT_R32_FLOAT;
		case ShaderDataType::Float2:	return DXGI_FORMAT_R32G32_FLOAT;
		case ShaderDataType::Float3:	return DXGI_FORMAT_R32G32B32_FLOAT;
		case ShaderDataType::Float4:	return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case ShaderDataType::Int:		return DXGI_FORMAT_R32_SINT;
		case ShaderDataType::Int2:		return DXGI_FORMAT_R32G32_SINT;
		case ShaderDataType::Int3:		return DXGI_FORMAT_R32G32B32_SINT;
		case ShaderDataType::Int4:		return DXGI_FORMAT_R32G32B32A32_SINT;
		case ShaderDataType::UInt:		return DXGI_FORMAT_R32_UINT;
		case ShaderDataType::UInt2:		return DXGI_FORMAT_R32G32_UINT;
		case ShaderDataType::UInt3:		return DXGI_FORMAT_R32G32B32_UINT;
		case ShaderDataType::UInt4:		return DXGI_FORMAT_R32G32B32A32_UINT;
		}
		EPOCH_ASSERT(false, "Unkown shader data type!");
		return DXGI_FORMAT_UNKNOWN;
	}

	static D3D_PRIMITIVE_TOPOLOGY ToDX11Topology(PrimitiveTopology aTopology)
	{
		switch (aTopology)
		{
		case PrimitiveTopology::PointList:		return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		case PrimitiveTopology::LineList:		return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		case PrimitiveTopology::TriangleList:	return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		}
		EPOCH_ASSERT(false, "Unkown topology!");
		return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	}

	static D3D11_COMPARISON_FUNC ToDX11CompareOperator(DepthCompareOperator aDepthCompareOperator)
	{
		switch (aDepthCompareOperator)
		{
		case Epoch::DepthCompareOperator::Never:			return D3D11_COMPARISON_NEVER;
		case Epoch::DepthCompareOperator::NotEqual:			return D3D11_COMPARISON_NOT_EQUAL;
		case Epoch::DepthCompareOperator::Less:				return D3D11_COMPARISON_LESS;
		case Epoch::DepthCompareOperator::LessOrEqual:		return D3D11_COMPARISON_LESS_EQUAL;
		case Epoch::DepthCompareOperator::Greater:			return D3D11_COMPARISON_GREATER;
		case Epoch::DepthCompareOperator::GreaterOrEqual:	return D3D11_COMPARISON_GREATER_EQUAL;
		case Epoch::DepthCompareOperator::Equal:			return D3D11_COMPARISON_EQUAL;
		case Epoch::DepthCompareOperator::Always:			return D3D11_COMPARISON_ALWAYS;
		}
		EPOCH_ASSERT(false, "Unkown depth compare operator!");
		return D3D11_COMPARISON_NEVER;
	}

	static D3D11_FILL_MODE ToDX11FillMode(RasterizerState aRasterizerState)
	{
		switch (aRasterizerState)
		{
		case RasterizerState::CullBack:		return D3D11_FILL_SOLID;
		case RasterizerState::CullNone:		return D3D11_FILL_SOLID;
		case RasterizerState::CullFront:	return D3D11_FILL_SOLID;
		case RasterizerState::Wireframe:	return D3D11_FILL_WIREFRAME;
		}
		EPOCH_ASSERT(false, "Unkown rasterizer state!");
		return D3D11_FILL_SOLID;
	}

	static D3D11_CULL_MODE ToDX11CullMode(RasterizerState aRasterizerState)
	{
		switch (aRasterizerState)
		{
		case RasterizerState::CullBack:		return D3D11_CULL_BACK;
		case RasterizerState::CullNone:		return D3D11_CULL_NONE;
		case RasterizerState::CullFront:	return D3D11_CULL_FRONT;
		case RasterizerState::Wireframe:	return D3D11_CULL_NONE;
		}
		EPOCH_ASSERT(false, "Unkown rasterizer state!");
		return D3D11_CULL_BACK;
	}

	class DX11RenderPipeline : public RenderPipeline
	{
	public:
		DX11RenderPipeline(const PipelineSpecification& aSpec);
		~DX11RenderPipeline() override = default;

		ComPtr<ID3D11InputLayout> GetInputLayout() const { return myInputLayout; }
		ComPtr<ID3D11RasterizerState> GetRasterizerState() const { return myRasterizerState; }
		ComPtr<ID3D11BlendState> GetBlendState() const { return myBlendState; }
		ComPtr<ID3D11DepthStencilState> GetDepthStencilState() const { return myDepthState; }

	private:
		ComPtr<ID3D11InputLayout> myInputLayout;
		ComPtr<ID3D11RasterizerState> myRasterizerState;
		ComPtr<ID3D11BlendState> myBlendState;
		ComPtr<ID3D11DepthStencilState> myDepthState;
	};
}
