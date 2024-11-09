#pragma once
#include "PhysicsScene.h"

namespace Epoch
{
	enum class PhysicsAPIType { PhysX };

	class PhysicsAPI
	{
	public:
		virtual ~PhysicsAPI() = default;
		
		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual std::shared_ptr<PhysicsScene> CreateScene(Scene* aScene) const = 0;
		
		static PhysicsAPIType Current() { return staticCurrentPhysicsAPI; }
		static void SetCurrentAPI(PhysicsAPIType aApi) { staticCurrentPhysicsAPI = aApi; }

	private:
		inline static PhysicsAPIType staticCurrentPhysicsAPI = PhysicsAPIType::PhysX;
	};
}
