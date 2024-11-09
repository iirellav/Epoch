#include "epch.h"
#include "PhysicsLayer.h"
#include "Epoch/Utils/ContainerUtils.h"

namespace Epoch
{
	uint32_t PhysicsLayerManager::AddLayer(const std::string& aName, bool aSetCollisions)
	{
		for (const auto& layer : staticLayers)
		{
			if (layer.name == aName)
			{
				return layer.layerID;
			}
		}

		uint32_t layerId = GetNextLayerID();
		PhysicsLayer layer = { layerId, aName, static_cast<int32_t>(BIT(layerId)), static_cast<int32_t>(BIT(layerId)) };
		staticLayers.insert(staticLayers.begin() + layerId, layer);
		staticLayerNames.insert(staticLayerNames.begin() + layerId, aName);

		if (aSetCollisions)
		{
			for (const auto& layer2 : staticLayers)
			{
				SetLayerCollision(layer.layerID, layer2.layerID, true);
			}
		}

		return layer.layerID;
	}

	void PhysicsLayerManager::RemoveLayer(uint32_t aLayerId)
	{
		PhysicsLayer& layerInfo = GetLayer(aLayerId);

		for (auto& otherLayer : staticLayers)
		{
			if (otherLayer.layerID == aLayerId)
			{
				continue;
			}

			if (otherLayer.collidesWith & layerInfo.bitValue)
			{
				otherLayer.collidesWith &= ~layerInfo.bitValue;
			}
		}

		Utils::RemoveIf(staticLayerNames, [&](const std::string& name) { return name == layerInfo.name; });
		Utils::RemoveIf(staticLayers, [&](const PhysicsLayer& layer) { return layer.layerID == aLayerId; });
	}

	void PhysicsLayerManager::UpdateLayerName(uint32_t aLayerId, const std::string& aNewName)
	{
		for (const auto& layerName : staticLayerNames)
		{
			if (layerName == aNewName)
			{
				return;
			}
		}

		PhysicsLayer& layer = GetLayer(aLayerId);
		Utils::RemoveIf(staticLayerNames, [&](const std::string& name) { return name == layer.name; });
		staticLayerNames.insert(staticLayerNames.begin() + aLayerId, aNewName);
		layer.name = aNewName;
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
			if (otherLayer.layerID == aLayerId)
			{
				continue;
			}

			if (layer.collidesWith & otherLayer.bitValue)
			{
				layers.push_back(otherLayer);
			}
		}

		return layers;
	}

	PhysicsLayer& PhysicsLayerManager::GetLayer(uint32_t aLayerId)
	{
		return aLayerId >= staticLayers.size() ? staticNullLayer : staticLayers[aLayerId];
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

	bool PhysicsLayerManager::IsLayerValid(uint32_t aLayerId)
	{
		const PhysicsLayer& layer = GetLayer(aLayerId);
		return layer.layerID != staticNullLayer.layerID && layer.IsValid();
	}

	bool PhysicsLayerManager::IsLayerValid(const std::string& aLayerName)
	{
		const PhysicsLayer& layer = GetLayer(aLayerName);
		return layer.layerID != staticNullLayer.layerID && layer.IsValid();
	}

	void PhysicsLayerManager::ClearLayers()
	{
		staticLayers.clear();
		staticLayerNames.clear();
	}

	uint32_t PhysicsLayerManager::GetNextLayerID()
	{
		int32_t lastId = -1;

		for (const auto& layer : staticLayers)
		{
			if (lastId != -1 && int32_t(layer.layerID) != lastId + 1)
			{
				return uint32_t(lastId + 1);
			}

			lastId = layer.layerID;
		}

		return (uint32_t)staticLayers.size();
	}
}