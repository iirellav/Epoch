#include "epch.h"
#include "Material.h"
#include "Epoch/Assets/AssetManager.h"

namespace Epoch
{
	CU::Vector3f& Material::GetAlbedoColor()
	{
		return myData.albedoColor;
	}

	void Material::SetAlbedoColor(const CU::Vector3f& aColor)
	{
		myData.albedoColor = aColor;
	}

	float& Material::GetMetalness()
	{
		return myData.metalness;
	}

	void Material::SetMetalness(float aValue)
	{
		myData.metalness = aValue;
	}

	float& Material::GetRoughness()
	{
		return myData.roughness;
	}

	void Material::SetRoughness(float aValue)
	{
		myData.roughness = aValue;
	}

	float& Material::GetNormalStrength()
	{
		return myData.normalStrength;
	}

	void Material::SetNormalStrength(float aValue)
	{
		myData.normalStrength = aValue;
	}

	float& Material::GetEmissionStrength()
	{
		return myData.emissionStrength;
	}

	void Material::SetEmissionStrength(float aValue)
	{
		myData.emissionStrength = aValue;
	}

	CU::Vector2f& Material::GetUVTiling()
	{
		return myData.uvTiling;
	}

	void Material::SetUVTiling(CU::Vector2f aUVTiling)
	{
		myData.uvTiling = aUVTiling;
	}

	CU::Vector3f& Material::GetEmissionColor()
	{
		return myData.emissionColor;
	}

	void Material::SetEmissionColor(const CU::Vector3f& aColor)
	{
		myData.emissionColor = aColor;
	}

	UUID& Material::GetAlbedoTexture()
	{
		return myAlbedoTexture;
	}

	void Material::SetAlbedoTexture(UUID aTextureID)
	{
		myAlbedoTexture = aTextureID;
	}

	void Material::ClearAlbedoTexture()
	{
		myAlbedoTexture = 0;
	}

	UUID& Material::GetNormalTexture()
	{
		return myNormalTexture;
	}

	void Material::SetNormalTexture(UUID aTextureID)
	{
		myNormalTexture = aTextureID;
	}

	void Material::ClearNormalTexture()
	{
		myNormalTexture = 0;
	}

	UUID& Material::GetMaterialTexture()
	{
		return myMaterialTexture;
	}

	void Material::SetMaterialTexture(UUID aTextureID)
	{
		myMaterialTexture = aTextureID;
	}

	void Material::ClearMaterialTexture()
	{
		myMaterialTexture = 0;
	}

	void Material::SetDefaults()
	{
		myData = Data();

		myAlbedoTexture = 0;
		myNormalTexture = 0;
		myMaterialTexture = 0;
	}


	MaterialTable::MaterialTable()
	{
		AddDefaultMaterial();
	}

	MaterialTable::MaterialTable(std::shared_ptr<MaterialTable> aOther)
	{
		myMaterials.resize(aOther->GetMaterialCount());

		for (uint32_t i = 0; i < aOther->GetMaterialCount(); i++)
		{
			SetMaterial(i, aOther->GetMaterial(i));
		}
	}

	void MaterialTable::SetMaterial(uint32_t aIndex, AssetHandle aMaterial)
	{
		if (aIndex >= myMaterials.size())
		{
			LOG_ERROR("Tried to set a material at an invalid index!");
			return;
		}

		myMaterials[aIndex] = aMaterial;
	}

	void MaterialTable::AddMaterial(AssetHandle aMaterial)
	{
		myMaterials.emplace_back(aMaterial);
	}

	void MaterialTable::AddDefaultMaterial()
	{
		myMaterials.emplace_back(Hash::GenerateFNVHash("Default-Material"));
	}

	void MaterialTable::RemoveMaterial()
	{
		if (myMaterials.empty())
		{
			return;
		}

		myMaterials.pop_back();
	}

	AssetHandle MaterialTable::GetMaterial(uint32_t aMaterialIndex) const
	{
		if (myMaterials.empty())
		{
			return 0;
		}

		aMaterialIndex = CU::Math::Clamp(aMaterialIndex, 0u, (uint32_t)myMaterials.size() - 1);
		return myMaterials[aMaterialIndex];
	}
}
