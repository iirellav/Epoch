#include "epch.h"
#include "DX11Shader.h"
#include "DX11ShaderCompiler.h"
#include "Epoch/Rendering/RHI.h" //TODO: Temp, remove
#include <d3dcommon.h>

namespace Epoch
{
	DX11Shader::DX11Shader(const std::string& aFilepath, const std::string& aName, bool aDisableOptimization)
	{
		myFilePath = aFilepath;

		myName = aName;
		myDisableOptimization = aDisableOptimization;
	}

	void DX11Shader::Release()
	{
		for (auto& [stage, shaderObj] : myShaderObjects)
		{
			shaderObj.Reset();
		}

		myShaderObjects.clear();
		myShaderData.clear();
	}

	static std::string ShaderNameExtensionFromStage(ShaderStage aStage)
	{
		switch (aStage)
		{
		case Epoch::ShaderStage::Vertex: return		"_VS";
		case Epoch::ShaderStage::Geometry: return	"_GS";
		case Epoch::ShaderStage::Pixel: return		"_PS";
		case Epoch::ShaderStage::Compute: return	"_CS";
		default: return "";
		}
	}

	bool DX11Shader::CreateShaders(const std::map<ShaderStage, std::vector<uint8_t>>& aShaderData)
	{
		EPOCH_PROFILE_FUNC();

		myShaderData = aShaderData;

		for (auto [stage, data] : myShaderData)
		{
			switch (stage)
			{
			case Epoch::ShaderStage::Vertex:
			{
				ComPtr<ID3D11VertexShader> shader;
				HRESULT result = RHI::GetDevice()->CreateVertexShader(myShaderData[stage].data(), myShaderData[stage].size(), nullptr, &shader);
				if (FAILED(result))
				{
					LOG_ERROR("Failed to create the vertex shader! '{}'", myName);
					return false;
				}

				myShaderObjects[stage] = shader;

				break;
			}
			case Epoch::ShaderStage::Geometry:
			{
				ComPtr<ID3D11GeometryShader> shader;
				HRESULT result = RHI::GetDevice()->CreateGeometryShader(myShaderData[stage].data(), myShaderData[stage].size(), nullptr, &shader);
				if (FAILED(result))
				{
					LOG_ERROR("Failed to create the geometry shader! '{}'", myName);
					return false;
				}

				myShaderObjects[stage] = shader;

				break;
			}
			case Epoch::ShaderStage::Pixel:
			{
				ComPtr<ID3D11PixelShader> shader;
				HRESULT result = RHI::GetDevice()->CreatePixelShader(myShaderData[stage].data(), myShaderData[stage].size(), nullptr, &shader);
				if (FAILED(result))
				{
					LOG_ERROR("Failed to create the pixel shader! '{}'", myName);
					return false;
				}

				myShaderObjects[stage] = shader;

				break;
			}
			case Epoch::ShaderStage::Compute:
			{
				ComPtr<ID3D11ComputeShader> shader;
				HRESULT result = RHI::GetDevice()->CreateComputeShader(myShaderData[stage].data(), myShaderData[stage].size(), nullptr, &shader);
				if (FAILED(result))
				{
					LOG_ERROR("Failed to create the compute shader! '{}'", myName);
					return false;
				}

				myShaderObjects[stage] = shader;

				break;
			}
			}

			const std::string name = myName + ShaderNameExtensionFromStage(stage);
			D3D_SET_OBJECT_NAME_A(myShaderObjects[stage], name.data());
		}

		return true;
	}
}
