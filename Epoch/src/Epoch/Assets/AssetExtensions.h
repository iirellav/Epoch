#pragma once
#include <unordered_map>
#include "AssetTypes.h"

namespace Epoch
{
	inline static std::unordered_map<std::string, AssetType> staticAssetExtensionMap =
	{
		// Epoch types
		{ ".epoch",		AssetType::Scene },
		{ ".mat",		AssetType::Material },
		{ ".anim",		AssetType::Animation },
		{ ".prefab",	AssetType::Prefab },
		
		// Script file
		{ ".cs",		AssetType::ScriptFile },

		// Mesh/Animation source
		{ ".fbx",		AssetType::Mesh },
		{ ".gltf",		AssetType::Mesh },
		{ ".glb",		AssetType::Mesh },
		{ ".obj",		AssetType::Mesh },

		// Texture
		{ ".png",		AssetType::Texture },
		{ ".jpg",		AssetType::Texture },
		{ ".jpeg",		AssetType::Texture },
		{ ".hdr",		AssetType::EnvTexture },

		//Font
		{ ".ttf",		AssetType::Font },

		// Audio
		{ ".wav",		AssetType::Audio },
		
		// Video
		{ ".mp4",		AssetType::Video }
	};
}
