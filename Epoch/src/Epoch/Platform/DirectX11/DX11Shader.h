#pragma once
#include <string>
#include "Epoch/Rendering/Shader.h"
#include <d3d11.h>
#include <wrl.h>

using namespace Microsoft::WRL;

namespace Epoch
{
	class DX11Shader : public Shader
	{
	public:
		DX11Shader(const std::string& aFilepath, const std::string& aName, bool aDisableOptimization);
		~DX11Shader() override = default;

		void Release();

		bool HasShader(ShaderStage aStage) { return myShaderData.find(aStage) != myShaderData.end(); }
		std::vector<uint8_t>& GetShaderData(ShaderStage aStage) { return myShaderData[aStage]; }
		ComPtr<ID3D11DeviceChild> GetShader(ShaderStage aStage) { return myShaderObjects[aStage]; }

	private:
		bool CreateShaders(const std::unordered_map<ShaderStage, std::vector<uint8_t>>& aShaderData);
		
	private:
		std::unordered_map<ShaderStage, std::vector<uint8_t>> myShaderData;
		std::unordered_map<ShaderStage, ComPtr<ID3D11DeviceChild>> myShaderObjects;

		friend class DX11ShaderCompiler;
	};
}
