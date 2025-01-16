#pragma once
#include <string_view>
#include "Epoch/Debug/Assert.h"

namespace Epoch
{
	enum class AssetType : uint16_t
	{
		None,
		Scene,
		Texture,
		Mesh,
		EnvTexture,
		Material,
		Audio,
		Video,
		Prefab,
		Script,
		ScriptFile,
		Font
	};

	inline AssetType AssetTypeFromString(std::string_view aAssetType)
	{
		if (aAssetType == "None")			return AssetType::None;
		if (aAssetType == "Scene")			return AssetType::Scene;
		if (aAssetType == "Texture")		return AssetType::Texture;
		if (aAssetType == "Mesh")			return AssetType::Mesh;
		if (aAssetType == "EnvTexture")		return AssetType::EnvTexture;
		if (aAssetType == "Material")		return AssetType::Material;
		if (aAssetType == "Audio")			return AssetType::Audio;
		if (aAssetType == "Video")			return AssetType::Video;
		if (aAssetType == "Prefab")			return AssetType::Prefab;
		if (aAssetType == "Script")			return AssetType::Script;
		if (aAssetType == "ScriptFile")		return AssetType::ScriptFile;
		if (aAssetType == "Font")			return AssetType::Font;
		
		EPOCH_ASSERT(false, "Unknown Asset Type");
		return AssetType::None;
	}

	inline const char* AssetTypeToString(AssetType aAssetType)
	{
		switch (aAssetType)
		{
		case AssetType::None:			return "None";
		case AssetType::Scene:			return "Scene";
		case AssetType::Texture:		return "Texture";
		case AssetType::Mesh:			return "Mesh";
		case AssetType::EnvTexture:		return "EnvTexture";
		case AssetType::Material:		return "Material";
		case AssetType::Audio:			return "Audio";
		case AssetType::Video:			return "Video";
		case AssetType::Prefab:			return "Prefab";
		case AssetType::Script:			return "Script";
		case AssetType::ScriptFile:		return "ScriptFile";
		case AssetType::Font:			return "Font";
		}

		EPOCH_ASSERT(false, "Unknown Asset Type");
		return "None";
	}
}