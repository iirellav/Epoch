#include "epch.h"
#include "PhysXAPI.h"
#include "Epoch/Core/JobSystem.h" 
#include "PhysXScene.h"

namespace Epoch
{
	void PhysXAPI::Init()
	{
		EPOCH_PROFILE_FUNC();

		myFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, myDefaultAllocator, myDefaultErrorCallback);
	
		myTolerancesScale.length = 1;
		myTolerancesScale.speed = 982;

		myPVD = physx::PxCreatePvd(*myFoundation);

		myPhysicsSystem = PxCreatePhysics(PX_PHYSICS_VERSION, *myFoundation, myTolerancesScale, true, myPVD);

		myDispatcher = physx::PxDefaultCpuDispatcherCreate(JobSystem::WorkerCount);
		myDefaultPhysicsMat = myPhysicsSystem->createMaterial(0.8f, 0.7f, 0.1f);
	}

	void PhysXAPI::Shutdown()
	{
		myPhysicsSystem->release();
		myPhysicsSystem = nullptr;
	}

	std::shared_ptr<PhysicsScene> PhysXAPI::CreateScene(Scene* aScene) const
	{
		return std::make_shared<PhysXScene>(aScene);
	}

	void PhysXAPI::ConnectPVD()
	{
		physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
		if (!myPVD->connect(*transport, physx::PxPvdInstrumentationFlag::eALL))
		{
			LOG_WARNING("Failed to connect to PhysX visual debugger");
		}
	}

	void PhysXAPI::DisconnectPVD()
	{
		myPVD->disconnect();
	}

	void PhysXAPI::InitControllerManager(PhysXScene* aScene)
	{
		myControllerManager = PxCreateControllerManager(*aScene->GetNative());
	}

	void PhysXAPI::ShutdownControllerManager()
	{
		myControllerManager->purgeControllers();
		myControllerManager->release();
	}
}
