#pragma once
#include "Epoch/Physics/CharacterController.h"
#include <characterkinematic/PxController.h>

namespace Epoch
{
	class PhysXCharacterController : public CharacterController
	{
	public:
		PhysXCharacterController(Entity aEntity);
		~PhysXCharacterController() override;

		physx::PxController* GetCharacterController() { return myCharacterController; }

		float GetStepOffset() override;
		void SetStepOffset(float aValue) override;
		
		float GetSlopeLimit() override;
		void SetSlopeLimit(float aValue) override;

		void Resize(float aHeight) override;

		void Move(const CU::Vector3f& aDisplacement) override;

		void Simulate(float aTimeStep) override;

	private:
		physx::PxController* myCharacterController;

		CU::Vector3f myDisplacement; // displacement (if any) for next update (comes from Move() calls)

		uint32_t myLayerValue;
		uint32_t myCollisionValue;
	};
}
