#include "epch.h"
#include "Shader.h"
#include "Epoch/Rendering/RendererAPI.h"
#include "Epoch/Platform/DirectX11/DX11Shader.h"
#include "Epoch/Platform/DirectX11/DX11ShaderCompiler.h"
#include "ShaderPack.h"

namespace Epoch
{
	std::shared_ptr<Shader> Shader::Create()
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::DirectX11: return std::make_shared<DX11Shader>();
		}
		return nullptr;
	}


	void ShaderLibrary::LoadShaderPack(const std::filesystem::path& aPath)
	{
		myShaderPack = std::make_shared<ShaderPack>(aPath);
		if (!myShaderPack->IsLoaded())
		{
			myShaderPack = nullptr;
			LOG_ERROR("Could not load shader pack: {}", aPath.string());
		}
	}

	void ShaderLibrary::Load(const std::filesystem::path& aShaderSourcePath, bool aDisableOptimization)
	{
		EPOCH_PROFILE_FUNC();

		std::shared_ptr<Shader> shader;

		switch (RendererAPI::Current())
		{
		case RendererAPIType::DirectX11:
		{
			if (myShaderPack)
			{
				if (myShaderPack->Contains(aShaderSourcePath.string()))
				{
					shader = myShaderPack->LoadShader(aShaderSourcePath.string());
				}
			}
			else
			{
				shader = DX11ShaderCompiler::Compile(aShaderSourcePath, aDisableOptimization);
			}
		}
		}

		EPOCH_ASSERT(shader, "Shader failed to compile/load!");
		const std::string& name = shader->GetName();
		if (Exists(name))
		{
			LOG_ERROR("Shader already loaded!");
		}
		
		std::lock_guard lock(myMutex);
		myShaders[name] = shader;
	}

	bool ShaderLibrary::Exists(const std::string& aName) const
	{
		return myShaders.find(aName) != myShaders.end();
	}

	std::shared_ptr<Shader> ShaderLibrary::Get(const std::string& aName)
	{
		EPOCH_ASSERT(Exists(aName), "Shader not found!");
		return myShaders[aName];
	}

	void ShaderLibrary::Reload(const std::string& aName)
	{
		EPOCH_PROFILE_FUNC();

		if (!Exists(aName))
		{
			LOG_WARNING("No shader with the name '{}' exists!", aName);
			return;
		}

		std::shared_ptr<Shader> shader = Get(aName);

		switch (RendererAPI::Current())
		{
		case RendererAPIType::DirectX11: DX11ShaderCompiler::Recompile(std::dynamic_pointer_cast<DX11Shader>(shader));
		}
	}
}
