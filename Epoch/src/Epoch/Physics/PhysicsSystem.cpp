#include "epch.h"
#include "PhysicsSystem.h"
#include "PhysX/PhysXAPI.h"
#include "PhysicsLayer.h"

namespace Epoch
{
	static PhysicsAPI* InitPhysicsAPI()
	{
		switch (PhysicsAPI::Current())
		{
			case PhysicsAPIType::PhysX: return new PhysXAPI();
		}

		EPOCH_ASSERT(false, "Unknown PhysicsAPI");
		return nullptr;
	}

	void PhysicsSystem::Init()
	{
		EPOCH_PROFILE_FUNC();

		staticPhysicsAPI = InitPhysicsAPI();
		staticPhysicsAPI->Init();

		PhysicsLayerManager::Init();
	}

	void PhysicsSystem::Shutdown()
	{
		staticPhysicsAPI->Shutdown();
		delete staticPhysicsAPI;
	}

	std::shared_ptr<PhysicsScene> PhysicsSystem::CreatePhysicsScene(Scene* aScene) { return staticPhysicsAPI->CreateScene(aScene); }
}