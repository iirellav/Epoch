#pragma once
#include <vector>
#include <string>
#include "Epoch/Assets/Asset.h"

namespace Epoch
{
	class Texture2D;

	class Material : public Asset
	{
	public:
		struct Data
		{
			CU::Vector3f albedoColor = CU::Vector3f::One;
			float normalStrength = 1;

			CU::Vector2f uvTiling = CU::Vector2f::One;
			float roughness = 1;
			float metalness = 0;
			
			CU::Vector3f emissionColor = CU::Vector3f::One;
			float emissionStrength = 0;
		};

	public:
		Material() = default;
		~Material() = default;

		CU::Vector3f& GetAlbedoColor();
		void SetAlbedoColor(const CU::Vector3f& aColor);

		float& GetMetalness();
		void SetMetalness(float aValue);

		float& GetRoughness();
		void SetRoughness(float aValue);

		float& GetNormalStrength();
		void SetNormalStrength(float aValue);

		CU::Vector2f& GetUVTiling();
		void SetUVTiling(CU::Vector2f aUVTiling);
		
		CU::Vector3f& GetEmissionColor();
		void SetEmissionColor(const CU::Vector3f& aColor);

		float& GetEmissionStrength();
		void SetEmissionStrength(float aValue);


		UUID& GetAlbedoTexture();
		void SetAlbedoTexture(UUID aTextureID);
		void ClearAlbedoTexture();

		UUID& GetNormalTexture();
		void SetNormalTexture(UUID aTextureID);
		void ClearNormalTexture();

		UUID& GetMaterialTexture();
		void SetMaterialTexture(UUID aTextureID);
		void ClearMaterialTexture();

		static AssetType GetStaticType() { return AssetType::Material; }
		AssetType GetAssetType() const override { return  GetStaticType(); }
		
	private:
		void SetDefaults();

	private:
		Data myData;

		UUID myAlbedoTexture = 0;
		UUID myNormalTexture = 0;
		UUID myMaterialTexture = 0;

		friend class MaterialSerializer;
	};

	class MaterialTable
	{
	public:
		MaterialTable();
		MaterialTable(std::shared_ptr<MaterialTable> aOther);
		~MaterialTable() = default;

		void SetMaterial(uint32_t aIndex, AssetHandle aMaterial);
		void AddMaterial(AssetHandle aMaterial);
		void AddDefaultMaterial();
		void RemoveMaterial();

		AssetHandle GetMaterial(uint32_t aMaterialIndex) const;
		uint32_t GetMaterialCount() const { return (uint32_t)myMaterials.size(); }

	private:
		std::vector<AssetHandle> myMaterials;
	};
}
