#pragma once
#include <memory>
#include "Texture.h"

namespace Epoch
{
	class Environment : public Asset
	{
	public:
		Environment() = default;
		Environment(const std::shared_ptr<TextureCube>& aRadianceMap, const std::shared_ptr<TextureCube>& aIrradianceMap) : myRadianceMap(aRadianceMap), myIrradianceMap(aIrradianceMap) {}
		~Environment() = default;

		std::shared_ptr<TextureCube> GetRadianceMap() { return myRadianceMap; }

		static AssetType GetStaticType() { return AssetType::EnvTexture; }
		AssetType GetAssetType() const override { return GetStaticType(); }

	private:
		std::shared_ptr<TextureCube> myRadianceMap;
		std::shared_ptr<TextureCube> myIrradianceMap;
	};
}
