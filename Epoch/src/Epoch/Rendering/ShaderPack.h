#pragma once
#include <filesystem>
#include "ShaderPackFile.h"

namespace Epoch
{
	class ShaderLibrary;
	class Shader;

	class ShaderPack
	{
	public:
		ShaderPack(const std::filesystem::path& aPath);

		bool IsLoaded() const { return myLoaded; }
		bool Contains(std::string_view aPath) const;

		std::shared_ptr<Shader> LoadShader(std::string_view aPath);

		static std::shared_ptr<ShaderPack> CreateFromLibrary(std::shared_ptr<ShaderLibrary> aShaderLibrary, const std::filesystem::path& aPath);

	private:
		bool myLoaded = false;
		ShaderPackFile myFile;
		std::filesystem::path myPath;
	};
}
