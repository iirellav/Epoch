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
		DX11Shader() = default;
		DX11Shader(const std::string& aFilepath, const std::string& aName, bool aDisableOptimization);
		~DX11Shader() override = default;

		void Release();

		ComPtr<ID3D11DeviceChild> GetShader(ShaderStage aStage) { return myShaderObjects[aStage]; }

	protected:
		bool CreateShaders(const std::map<ShaderStage, std::vector<uint8_t>>& aShaderData) override;
		
	private:
		std::map<ShaderStage, ComPtr<ID3D11DeviceChild>> myShaderObjects;

		friend class DX11ShaderCompiler;
	};
}
