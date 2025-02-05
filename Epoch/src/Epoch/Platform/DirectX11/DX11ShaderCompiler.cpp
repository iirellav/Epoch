#include "epch.h"
#include "DX11ShaderCompiler.h"
#include "DX11Shader.h"
#include "Epoch/Core/Application.h"

#include "Epoch/Rendering/RHI.h" //TODO: Remove
#include <d3dcompiler.h>

namespace Epoch
{
	DX11ShaderCompiler::DX11ShaderCompiler(const std::filesystem::path& aShaderSourcePath, bool aDisableOptimization)
		: myShaderSourcePath(aShaderSourcePath), myDisableOptimization(aDisableOptimization)
	{
		myLanguage = ShaderLangFromExtension(aShaderSourcePath.extension().string());
	}

	std::shared_ptr<DX11Shader> DX11ShaderCompiler::Compile(const std::filesystem::path& aShaderSourcePath, bool aDisableOptimization)
	{
		Timer timer;

		std::shared_ptr<DX11Shader> shader = std::make_shared<DX11Shader>(aShaderSourcePath.string(), aShaderSourcePath.stem().string(), aDisableOptimization);
		std::shared_ptr<DX11ShaderCompiler> compiler = std::make_shared<DX11ShaderCompiler>(aShaderSourcePath, aDisableOptimization);

		if (!compiler->Compile())
		{
			return nullptr;
		}

		if (!shader->CreateShaders(compiler->myBinaries))
		{
			return nullptr;
		}

		LOG_INFO("Successfully compiled and created {} shaders from '{}' Time: {}ms", compiler->myBinaries.size(), aShaderSourcePath.string(), timer.ElapsedMillis());
		return shader;
	}

	bool DX11ShaderCompiler::Recompile(std::shared_ptr<DX11Shader> aShader)
	{
		Timer timer;

		std::shared_ptr<DX11ShaderCompiler> compiler = std::make_shared<DX11ShaderCompiler>(aShader->myFilePath, aShader->myDisableOptimization);

		if (!compiler->Compile(true))
		{
			return false;
		}

		aShader->Release();

		if (!aShader->CreateShaders(compiler->myBinaries))
		{
			return false;
		}

		LOG_INFO("Successfully recompiled and created {} shaders from '{}' Time: {}ms", compiler->myBinaries.size(), aShader->myFilePath, timer.ElapsedMillis());
		return true;
	}

	static std::filesystem::path GetCacheDirectory()
	{
		return Application::Get().GetSpecification().cacheDirectory + "/Shader";
	}

	static void CreateCacheDirectoryIfNeeded()
	{
		auto cacheDirectory = GetCacheDirectory();
		if (!std::filesystem::exists(cacheDirectory))
		{
			std::filesystem::create_directories(cacheDirectory);
		}
	}

	inline static const char* ShaderStageCachedFileExtension(ShaderStage aStage)
	{
		switch (aStage)
		{
		case ShaderStage::Vertex:	return ".cached.vert";
		case ShaderStage::Geometry:	return ".cached.geo";
		case ShaderStage::Pixel:	return ".cached.frag";
		case ShaderStage::Compute:	return ".cached.comp";
		}

		EPOCH_ASSERT(false, "Unknown shader stage!");
		return "";
	}

	inline static const char* Target(ShaderStage aStage)
	{
		switch (aStage)
		{
		case ShaderStage::Vertex:	return "vs_5_0";
		case ShaderStage::Geometry:	return "gs_5_0";
		case ShaderStage::Pixel:	return "ps_5_0";
		case ShaderStage::Compute:	return "cs_5_0";
		}

		EPOCH_ASSERT(false, "Unknown shader stage!");
		return "";
	}

	bool DX11ShaderCompiler::Compile(bool aForceCompile)
	{
		EPOCH_PROFILE_FUNC();

		myShaderSources.clear();

		CreateCacheDirectoryIfNeeded();

		std::string source = CU::ReadFileAndSkipBOM(myShaderSourcePath);
		if (source.empty())
		{
			LOG_ERROR("Failed to read file '{}'!", myShaderSourcePath.string());
			return false;
		}

		if (!PreProcess(source))
		{
			LOG_ERROR("Failed to pre process shader '{}'!", myShaderSourcePath.string());
			return false;
		}

		if (!CompileOrGetDX11Binaries(aForceCompile))
		{
			return false;
		}

		return true;
	}

	bool DX11ShaderCompiler::PreProcess(const std::string& aSource)
	{
		EPOCH_PROFILE_FUNC();

		const char* stageToken = "#stage";
		size_t stageTokenLength = strlen(stageToken);
		size_t pos = aSource.find(stageToken, 0); //Start of shader stage declaration line
		while (pos != std::string::npos)
		{
			size_t eol = aSource.find_first_of("\r\n", pos); //End of shader stage declaration line
			EPOCH_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + stageTokenLength + 1; //Start of shader stage name (after "#stage " keyword)
			std::string stage = aSource.substr(begin, eol - begin);
			EPOCH_ASSERT(ShaderTypeFromString(stage) != ShaderStage::None, "Invalid shader stage specified");

			size_t nextLinePos = aSource.find_first_not_of("\r\n", eol); //Start of shader code after shader stage declaration line
			EPOCH_ASSERT(nextLinePos != std::string::npos, "Syntax error");
			pos = aSource.find(stageToken, nextLinePos); //Start of next shader stage declaration line

			myShaderSources[ShaderTypeFromString(stage)] = (pos == std::string::npos) ? aSource.substr(nextLinePos) : aSource.substr(nextLinePos, pos - nextLinePos);
		}

		return true;
	}

	bool DX11ShaderCompiler::CompileOrGetDX11Binaries(bool aForceCompile)
	{
		for (auto [stage, source] : myShaderSources)
		{
			if (!CompileOrGetDX11Binary(stage, aForceCompile))
			{
				return false;
			}
		}
		
		return true;
	}

	bool DX11ShaderCompiler::CompileOrGetDX11Binary(ShaderStage aStage, bool aForceCompile)
	{
		if (!aForceCompile)
		{
			TryGetCachedBinary(aStage);
		}

		if (myBinaries[aStage].empty())
		{
			if (CompileBinary(aStage))
			{
				const std::string extension = ShaderStageCachedFileExtension(aStage);
				const auto path = GetCacheDirectory() / (myShaderSourcePath.filename().string() + extension);
				const std::string cachedFilePath = path.string();

				FILE* f = fopen(cachedFilePath.c_str(), "wb");
				if (!f)
				{
					LOG_ERROR("Failed to cache shader binary!");
				}
				else
				{
					fwrite(myBinaries[aStage].data(), sizeof(uint8_t), myBinaries[aStage].size(), f);
					fclose(f);
				}
			}
			else
			{
				LOG_ERROR("Failed to compile shader binaries '{}'!", myShaderSourcePath.string());
				return false;
			}
		}

		return true;
	}

	void DX11ShaderCompiler::TryGetCachedBinary(ShaderStage aStage)
	{
		const std::string extension = ShaderStageCachedFileExtension(aStage);
		const auto path = GetCacheDirectory() / (myShaderSourcePath.filename().string() + extension);
		const std::string cachedFilePath = path.string();

		if (FileSystem::Exists(cachedFilePath) && FileSystem::IsNewer(myShaderSourcePath, cachedFilePath))
		{
			return;
		}

		FILE* f = fopen(cachedFilePath.data(), "rb");
		if (!f)
		{
			return;
		}

		fseek(f, 0, SEEK_END);
		uint64_t size = ftell(f);
		fseek(f, 0, SEEK_SET);
		myBinaries[aStage] = std::vector<uint8_t>(size);
		fread(myBinaries[aStage].data(), sizeof(uint8_t), myBinaries[aStage].size(), f);
		fclose(f);
	}

	bool DX11ShaderCompiler::CompileBinary(ShaderStage aStage)
	{
		EPOCH_PROFILE_FUNC();

		const std::string& source = myShaderSources[aStage];

		ComPtr<ID3DBlob> blob;
		ComPtr<ID3DBlob> errorBlob;
		HRESULT result = D3DCompile(
			source.c_str(),													//pSrcData
			source.length(),												//SrcDataSize
			myShaderSourcePath.string().c_str(),							//pSourceName
			NULL,															//*pDefines
			D3D_COMPILE_STANDARD_FILE_INCLUDE,								//*pInclude
			"main",															//pEntrypoint
			Target(aStage),													//pTarget
			D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3,	//Flags1
			NULL,															//Flags2
			&blob,															//**ppCode
			&errorBlob);													//**ppErrorMsgs

		if (FAILED(result))
		{
			LOG_ERROR("Failed to compile {} - {} shader!\nError: {}", myShaderSourcePath.stem().string(), ShaderTypeToString(aStage), (char*)errorBlob->GetBufferPointer());
			CONSOLE_LOG_ERROR("Failed to compile {} - {} shader!\nError: {}", myShaderSourcePath.stem().string(), ShaderTypeToString(aStage), (char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
			if (blob)
			{
				blob->Release();
			}
			return false;
		}
		else
		{
			auto& binaries = myBinaries[aStage];
			const size_t size = blob->GetBufferSize();
			binaries.resize(size);
			std::memcpy(binaries.data(), blob->GetBufferPointer(), size);
			blob->Release();
		}

		return true;
	}
}