#pragma once
#include <memory>
#include "PhysicsAPI.h"
#include "PhysicsScene.h"
#include "PhysicsSettings.h"

namespace Epoch
{
	class PhysicsSystem
	{
	public:
		static void Init();
		static void Shutdown();
		
		static PhysicsSettings& GetSettings() { return staticPhysicsSettings; }

		static std::shared_ptr<PhysicsScene> CreatePhysicsScene(Scene* aScene);

		static PhysicsAPI* GetAPI() { return staticPhysicsAPI; }
		
	private:
		inline static PhysicsAPI* staticPhysicsAPI;
		inline static PhysicsSettings staticPhysicsSettings;
	};
}
