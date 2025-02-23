#pragma once
#include "Epoch/Physics/PhysicsAPI.h"

#include <PxPhysics.h>
#include <PxPhysicsAPI.h>
#include <characterkinematic/PxControllerManager.h>

namespace Epoch
{
	class PhysXScene;

	class PhysXAPI : public PhysicsAPI
	{
	public:
		void Init() override;
		void Shutdown() override;

		std::shared_ptr<PhysicsScene> CreateScene(Scene* aScene) const override;

		physx::PxFoundation* GetFoundation() { return myFoundation; }
		physx::PxPhysics* GetPhysicsSystem() { return myPhysicsSystem; }
		physx::PxControllerManager* GetControllerManager() { return myControllerManager; }
		physx::PxDefaultCpuDispatcher* GetDispatcher() { return myDispatcher; }

		physx::PxMaterial* GetDefaultMaterial() { return myDefaultPhysicsMat; }
		physx::PxMaterial* GetMaterial(AssetHandle aAssetHandle);

		void ConnectPVD();
		void DisconnectPVD();
		
		void InitControllerManager(PhysXScene* aScene);
		void ShutdownControllerManager();
		void ClearMaterials();

	private:
		physx::PxFoundation* myFoundation;
		physx::PxPvd* myPVD = nullptr;
		physx::PxPhysics* myPhysicsSystem = nullptr;
		physx::PxControllerManager* myControllerManager;

		physx::PxTolerancesScale myTolerancesScale;
		physx::PxDefaultAllocator myDefaultAllocator;
		physx::PxDefaultErrorCallback myDefaultErrorCallback;

		physx::PxDefaultCpuDispatcher* myDispatcher = nullptr;

		physx::PxMaterial* myDefaultPhysicsMat = nullptr;
		std::unordered_map<AssetHandle, physx::PxMaterial*> myMaterialMap;
	};
}
