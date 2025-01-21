#include "epch.h"
#include "PhysicsLayer.h"
#include "Epoch/Utils/ContainerUtils.h"

namespace Epoch
{
	void PhysicsLayerManager::Init()
	{
		for (size_t i = 0; i < staticLayers.size(); i++)
		{
			staticLayers[i].layerID = (uint32_t)i;
			staticLayers[i].bitValue = BIT(i);
			SetLayerCollision((uint32_t)i, (uint32_t)i, true);
		}

		{
			staticLayers[0].name = "Default";
			staticLayers[0].reserved = true;
			staticLayerNames[0] = "Default";
		}
	}

	void PhysicsLayerManager::UpdateLayerName(uint32_t aLayerId, const std::string& aNewName)
	{
		if (aNewName != "")
		{
			for (const auto& layerName : staticLayerNames)
			{
				if (layerName == aNewName)
				{
					return;
				}
			}
		}

		PhysicsLayer& layer = GetLayer(aLayerId);
		layer.name = aNewName;
		staticLayerNames[aLayerId] = aNewName;

		if (aNewName == "")
		{
			for (const auto& collidingLayer : PhysicsLayerManager::GetLayerCollisions(aLayerId))
			{
				if (collidingLayer.layerID == aLayerId)
				{
					continue;
				}

				SetLayerCollision(aLayerId, collidingLayer.layerID, false);
			}
		}
	}

	void PhysicsLayerManager::SetLayerCollision(uint32_t aLayerId, uint32_t aOtherLayer, bool aShouldCollide)
	{
		if (ShouldCollide(aLayerId, aOtherLayer) && aShouldCollide)
		{
			return;
		}

		PhysicsLayer& layerInfo = GetLayer(aLayerId);
		PhysicsLayer& otherLayerInfo = GetLayer(aOtherLayer);

		if (aShouldCollide)
		{
			layerInfo.collidesWith |= otherLayerInfo.bitValue;
			otherLayerInfo.collidesWith |= layerInfo.bitValue;
		}
		else
		{
			layerInfo.collidesWith &= ~otherLayerInfo.bitValue;
			otherLayerInfo.collidesWith &= ~layerInfo.bitValue;
		}
	}

	std::vector<PhysicsLayer> PhysicsLayerManager::GetLayerCollisions(uint32_t aLayerId)
	{
		const PhysicsLayer& layer = GetLayer(aLayerId);

		std::vector<PhysicsLayer> layers;
		for (const auto& otherLayer : staticLayers)
		{
			if (layer.collidesWith & otherLayer.bitValue)
			{
				layers.push_back(otherLayer);
			}
		}

		return layers;
	}

	PhysicsLayer& PhysicsLayerManager::GetLayer(uint32_t aLayerId)
	{
		return aLayerId >= staticLayers.size() ? staticNullLayer : staticLayers[aLayerId].name == "" ? staticNullLayer : staticLayers[aLayerId];
	}

	PhysicsLayer& PhysicsLayerManager::GetLayer(const std::string& aLayerName)
	{
		for (auto& layer : staticLayers)
		{
			if (layer.name == aLayerName)
			{
				return layer;
			}
		}

		return staticNullLayer;
	}

	bool PhysicsLayerManager::ShouldCollide(uint32_t aLayer0, uint32_t aLayer1)
	{
		return GetLayer(aLayer0).collidesWith & GetLayer(aLayer1).bitValue;
	}
}