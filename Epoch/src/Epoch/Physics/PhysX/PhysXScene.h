#pragma once
#include "Epoch/Physics/PhysicsScene.h"
#include "PhysXEventCallback.h"

#include <PxPhysics.h>
#include <PxPhysicsAPI.h>

namespace Epoch
{
	class PhysXScene : public PhysicsScene
	{
	public:
		PhysXScene(Scene* aScene);
		~PhysXScene() override;

		void Destroy() override;
		void Simulate() override;

		physx::PxScene* GetNative() { return myScene; }

		std::shared_ptr<PhysicsBody> CreateBody(Entity aEntity) override;
		void DestroyBody(Entity aEntity) override;
		std::shared_ptr<CharacterController> CreateCharacterController(Entity aEntity) override;

		CU::Vector3f GetGravity() const override;
		void SetGravity(const CU::Vector3f& aGravity) override;

		bool Raycast(CU::Vector3f aOrigin, CU::Vector3f aDirection, float aMaxDistance = FLT_MAX, HitInfo* outHitInfo = nullptr) override;

		void AddRadialImpulse(CU::Vector3f aOrigin, float aRadius, float aStrength) override; //TODO: Move to PhysicsScene when Iv'e made a wrapper for everything

		void Teleport(Entity aEntity, const CU::Vector3f& aTargetPosition, const CU::Quatf& aTargetRotation) override;

	private:
		physx::PxScene* myScene = nullptr;

		std::unique_ptr<PhysXEventCallback> myEventCallback;
	};
}
