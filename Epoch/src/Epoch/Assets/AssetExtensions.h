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
		{ ".prefab",	AssetType::Prefab },
		
		// Script file
		{ ".cs",		AssetType::ScriptFile },

		// Mesh/Animation source
		{ ".fbx",		AssetType::Model },
		{ ".gltf",		AssetType::Model },
		{ ".glb",		AssetType::Model },
		{ ".obj",		AssetType::Model },

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
