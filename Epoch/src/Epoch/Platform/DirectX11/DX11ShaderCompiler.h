#pragma once
#include <memory>
#include <string>
#include <filesystem>
#include <unordered_map>
#include "Epoch/Rendering/Shader.h"

namespace Epoch
{
	class DX11Shader;

	class DX11ShaderCompiler
	{
	public:
		DX11ShaderCompiler(const std::filesystem::path& aShaderSourcePath, bool aDisableOptimization = false);
		~DX11ShaderCompiler() = default;

		static std::shared_ptr<DX11Shader> Compile(const std::filesystem::path& aShaderSourcePath, bool aDisableOptimization = false);
		static bool Recompile(std::shared_ptr<DX11Shader> aShader);
		
	private:
		bool Compile(bool aForceCompile = false);
		bool PreProcess(const std::string& aSource);

		bool CompileOrGetDX11Binaries(bool aForceCompile);
		bool CompileOrGetDX11Binary(ShaderStage aStage, bool aForceCompile);

		void TryGetCachedBinary(ShaderStage aStage);
		bool CompileBinary(ShaderStage aStage);

	private:
		std::filesystem::path myShaderSourcePath;
		bool myDisableOptimization = false;
		ShaderSourceLang myLanguage;

		std::unordered_map<ShaderStage, std::string> myShaderSources;
		std::unordered_map<ShaderStage, std::vector<uint8_t>> myBinaries;
	};
}
