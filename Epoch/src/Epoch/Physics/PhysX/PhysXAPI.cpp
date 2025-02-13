#include "epch.h"
#include "PhysXAPI.h"
#include "Epoch/Core/JobSystem.h" 
#include "PhysXScene.h"
#include "Epoch/Assets/AssetManager.h"
#include "Epoch/Physics/PhysicsMaterial.h"

namespace Epoch
{
	void PhysXAPI::Init()
	{
		EPOCH_PROFILE_FUNC();

		myFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, myDefaultAllocator, myDefaultErrorCallback);
	
		myTolerancesScale.length = 100;
		myTolerancesScale.speed = 982;
		
#ifndef _DIST
		myPVD = physx::PxCreatePvd(*myFoundation);
#endif

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

	physx::PxMaterial* PhysXAPI::GetMaterial(AssetHandle aAssetHandle)
	{
		if (aAssetHandle == 0)
		{
			return myDefaultPhysicsMat;
		}

		if (myMaterialMap.find(aAssetHandle) != myMaterialMap.end())
		{
			return myMaterialMap.at(aAssetHandle);
		}

		auto physMatAsset = AssetManager::GetAsset<PhysicsMaterial>(aAssetHandle);
		if (physMatAsset)
		{
			auto* physicsMaterial = myPhysicsSystem->createMaterial(physMatAsset->StaticFriction(), physMatAsset->DynamicFriction(), physMatAsset->Restitution());
			myMaterialMap.emplace(aAssetHandle, physicsMaterial);

			return physicsMaterial;
		}
		
		return myDefaultPhysicsMat;
	}

	void PhysXAPI::ConnectPVD()
	{
#ifndef _DIST
		if (!myPVD || myPVD->isConnected())
		{
			return;
		}

		physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
		if (!myPVD->connect(*transport, physx::PxPvdInstrumentationFlag::eALL))
		{
			LOG_WARNING("Failed to connect to PhysX visual debugger");
		}
#endif
	}

	void PhysXAPI::DisconnectPVD()
	{
#ifndef _DIST
		if (!myPVD || !myPVD->isConnected())
		{
			return;
		}

		myPVD->disconnect();
#endif
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

	void PhysXAPI::ClearMaterials()
	{
		myMaterialMap.clear();
	}
}
