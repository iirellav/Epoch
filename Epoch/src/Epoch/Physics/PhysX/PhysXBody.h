#pragma once
#include "Epoch/Physics/PhysicsBody.h"
#include <PxActor.h>

namespace Epoch
{
	class PhysXBody : public PhysicsBody
	{
	public:
		PhysXBody(Entity aEntity);
		~PhysXBody() override;
		
		float GetMass() override;
		void SetMass(float aMass) override;

		bool GetUseGravity() override;
		void SetUseGravity(bool aState) override;

		float GetDrag() override;
		void SetDrag(float aDrag) override;
		
		float GetAngularDrag() override;
		void SetAngularDrag(float aDrag) override;

		CU::Vector3f GetVelocity() override;
		void SetVelocity(CU::Vector3f aVelocity) override;
		
		CU::Vector3f GetAngularVelocity() override;
		void SetAngularVelocity(CU::Vector3f aVelocity) override;

		CU::Vector3f GetPosition() override;
		void SetPosition(CU::Vector3f aPosition) override;

		CU::Vector3f GetRotation() override;
		void SetRotation(CU::Vector3f aRotation) override;

		PhysicsAxis GetConstraints() override;
		void SetConstraints(PhysicsAxis aConstraints) override;

		void AddForce(CU::Vector3f aForce, ForceMode aForceMode = ForceMode::Force) override;
		void AddForceAtPosition(CU::Vector3f aForce, CU::Vector3f aPosition, ForceMode aForceMode = ForceMode::Force) override;

		void AddTorque(CU::Vector3f aTorque, ForceMode aForceMode = ForceMode::Force) override;

		void AddRadialImpulse(CU::Vector3f aOrigin, float aRadius, float aStrength) override;

	private:
		physx::PxActor* myActor = nullptr;

		friend class PhysXScene;
	};
}
