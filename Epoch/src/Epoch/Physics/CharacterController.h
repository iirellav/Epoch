#pragma once
#include "Epoch/Scene/Entity.h"

namespace Epoch
{
	class CharacterController
	{
	public:
		CharacterController() = default;
		virtual ~CharacterController() = default;

		virtual float GetStepOffset() = 0;
		virtual void SetStepOffset(float aValue) = 0;

		virtual float GetSlopeLimit() = 0;
		virtual void SetSlopeLimit(float aValue) = 0;

		virtual void Resize(float aHeight) = 0;

		virtual void Move(const CU::Vector3f& aDisplacement) = 0;

		virtual void Simulate(float aTimeStep) = 0;
	};
}
