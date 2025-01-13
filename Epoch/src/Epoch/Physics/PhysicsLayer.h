#pragma once
#include <string>
#include <vector>

namespace Epoch
{
	struct LayerMask
	{
		int32_t bitValue = 0;
	};

	struct PhysicsLayer
	{
		uint32_t layerID = 0;
		std::string name = "";
		int32_t bitValue = 0;
		int32_t collidesWith = 0;
		bool collidesWithSelf = true;

		bool IsValid() const
		{
			return !name.empty() && bitValue > 0;
		}
	};

	class PhysicsLayerManager
	{
	public:
		static uint32_t AddLayer(const std::string& aName, bool aSetCollisions = true);
		static void RemoveLayer(uint32_t aLayerId);

		static void UpdateLayerName(uint32_t aLayerId, const std::string& aNewName);

		static void SetLayerCollision(uint32_t aLayerId, uint32_t aOtherLayer, bool aShouldCollide);
		static std::vector<PhysicsLayer> GetLayerCollisions(uint32_t aLayerId);

		static const std::vector<PhysicsLayer>& GetLayers() { return staticLayers; }
		static const std::vector<std::string>& GetLayerNames() { return staticLayerNames; }

		static PhysicsLayer& GetLayer(uint32_t aLayerId);
		static PhysicsLayer& GetLayer(const std::string& aLayerName);
		static uint32_t GetLayerCount() { return uint32_t(staticLayers.size()); }

		static bool ShouldCollide(uint32_t aLayer0, uint32_t aLayer1);
		static bool IsLayerValid(uint32_t aLayerId);
		static bool IsLayerValid(const std::string& aLayerName);

		static void ClearLayers();

	private:
		static uint32_t GetNextLayerID();

	private:
		static std::vector<PhysicsLayer> staticLayers;
		static std::vector<std::string> staticLayerNames;
		static PhysicsLayer staticNullLayer;
	};
}
