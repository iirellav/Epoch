#pragma once
#include <string>
#include <array>

namespace Epoch
{
	struct LayerMask
	{
		uint32_t bitValue = 0;
	};

	struct PhysicsLayer
	{
		uint32_t layerID = 0;
		std::string name = "";
		uint32_t bitValue = 0;
		uint32_t collidesWith = 0;

		bool reserved = false;

		bool IsValid() const
		{
			return !name.empty() && bitValue > 0;
		}
	};

	class PhysicsLayerManager
	{
	public:
		static void Init();

		static void UpdateLayerName(uint32_t aLayerId, const std::string& aNewName);

		static void SetLayerCollision(uint32_t aLayerId, uint32_t aOtherLayer, bool aShouldCollide);
		static std::vector<PhysicsLayer> GetLayerCollisions(uint32_t aLayerId);

		static const std::array<PhysicsLayer, 32>& GetLayers() { return staticLayers; }
		static const std::array<std::string, 32>& GetLayerNames() { return staticLayerNames; }

		static PhysicsLayer& GetLayer(uint32_t aLayerId);
		static PhysicsLayer& GetLayer(const std::string& aLayerName);

		static bool ShouldCollide(uint32_t aLayer0, uint32_t aLayer1);

	private:
		static inline std::array<PhysicsLayer, 32> staticLayers;
		static inline std::array<std::string, 32> staticLayerNames;
		static inline PhysicsLayer staticNullLayer;
	};
}
