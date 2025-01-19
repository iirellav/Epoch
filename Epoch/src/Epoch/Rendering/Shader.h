#pragma once
#include <string>
#include <filesystem>
#include <memory>
#include <map>
#include <unordered_map>
#include <wrl.h>
#include "Epoch/Core/Hash.h"

using namespace Microsoft::WRL;

struct ID3D11DeviceChild;

namespace Epoch
{
	enum class ShaderStage : uint8_t
	{
		None, Vertex, Geometry, Pixel, Compute
	};

	inline static ShaderStage ShaderTypeFromString(const std::string_view aType)
	{
		if (aType == "vertex")
		{
			return ShaderStage::Vertex;
		}

		if (aType == "geometry")
		{
			return ShaderStage::Geometry;
		}

		if (aType == "pixel" || aType == "fragment")
		{
			return ShaderStage::Pixel;
		}

		if (aType == "compute")
		{
			return ShaderStage::Compute;
		}
		
		EPOCH_ASSERT(false, "Unknown shader type!");
		return ShaderStage::None;
	}

	inline static std::string ShaderTypeToString(ShaderStage aType)
	{
		if (aType == ShaderStage::Vertex)
		{
			return "Vertex";
		}

		if (aType == ShaderStage::Geometry)
		{
			return "Geometry";
		}

		if (aType == ShaderStage::Pixel)
		{
			return "Pixel/Fragment";
		}

		if (aType == ShaderStage::Compute)
		{
			return "Compute";
		}
		
		EPOCH_ASSERT(false, "Unknown shader type!");
		return "Unknown";
	}

	enum class ShaderSourceLang
	{
		None, GLSL, HLSL
	};

	inline static ShaderSourceLang ShaderLangFromExtension(const std::string_view aExtension)
	{
		if (aExtension == ".glsl")	return ShaderSourceLang::GLSL;
		if (aExtension == ".hlsl")	return ShaderSourceLang::HLSL;
		
		EPOCH_ASSERT(false, "Unknown shader languish!");
		return ShaderSourceLang::None;
	}

	class Shader
	{
	public:
		Shader() = default;
		virtual ~Shader() = default;

		bool HasShader(ShaderStage aStage) { return myShaderData.find(aStage) != myShaderData.end(); }
		std::vector<uint8_t>& GetShaderData(ShaderStage aStage) { return myShaderData[aStage]; }

		const std::string& GetName() const { return myName; }
		const std::string& GetFilePath() const { return myFilePath; }

		uint32_t GetHash() { return (uint32_t)Hash::GenerateFNVHash(myFilePath); }

		static std::shared_ptr<Shader> Create();

	protected:
		virtual bool CreateShaders(const std::map<ShaderStage, std::vector<uint8_t>>& aShaderData) = 0;

	protected:
		std::string myName = "";
		bool myDisableOptimization = false;

		std::map<ShaderStage, std::vector<uint8_t>> myShaderData;

		std::string myFilePath = ""; //TEMP: Shader should be an asset

	private:
		friend class ShaderPack;
	};

	class ShaderLibrary
	{
	public:
		void LoadShaderPack(const std::filesystem::path& aPath);
		void Load(const std::filesystem::path& aShaderSourcePath, bool aDisableOptimization = false);

		bool Exists(const std::string& aName) const;
		std::shared_ptr<Shader> Get(const std::string& aName);
		void Reload(const std::string& aName);
		
		std::unordered_map<std::string, std::shared_ptr<Shader>>& GetShaders() { return myShaders; }
		const std::unordered_map<std::string, std::shared_ptr<Shader>>& GetShaders() const { return myShaders; }

	private:
		std::unordered_map<std::string, std::shared_ptr<Shader>> myShaders;
		std::shared_ptr<ShaderPack> myShaderPack;
	};
}