#pragma once
#include "Epoch/Scene/Entity.h"
#include "PhysicsShapes.h"

namespace Epoch
{
	class PhysicsBody
	{
	public:
		PhysicsBody(Entity aEntity);
		virtual ~PhysicsBody() = default;

		bool IsValid() const { return myShapes.size() > 0; }
		bool IsStatic() const { return myIsStatic; }

		virtual float GetMass() = 0;
		virtual void SetMass(float aMass) = 0;
		
		virtual bool GetUseGravity() = 0;
		virtual void SetUseGravity(bool aState) = 0;
		
		virtual float GetDrag() = 0;
		virtual void SetDrag(float aDrag) = 0;
		
		virtual float GetAngularDrag() = 0;
		virtual void SetAngularDrag(float aDrag) = 0;

		virtual CU::Vector3f GetVelocity() = 0;
		virtual void SetVelocity(CU::Vector3f aVelocity) = 0;

		virtual CU::Vector3f GetAngularVelocity() = 0;
		virtual void SetAngularVelocity(CU::Vector3f aVelocity) = 0;

		virtual CU::Vector3f GetPosition() = 0;
		virtual void SetPosition(CU::Vector3f aPosition) = 0;

		virtual CU::Vector3f GetRotation() = 0;
		virtual void SetRotation(CU::Vector3f aRotation) = 0;
		
		virtual PhysicsAxis GetConstraints() = 0;
		virtual void SetConstraints(PhysicsAxis aConstraints) = 0;

		virtual void AddForce(CU::Vector3f aForce, ForceMode aForceMode = ForceMode::Force) = 0;
		virtual void AddForceAtPosition(CU::Vector3f aForce, CU::Vector3f aPosition, ForceMode aForceMode = ForceMode::Force) = 0;

		virtual void AddTorque(CU::Vector3f aTorque, ForceMode aForceMode = ForceMode::Force) = 0;

		virtual void AddRadialImpulse(CU::Vector3f aOrigin, float aRadius, float aStrength) = 0;

	protected:
		void CreateCollisionShapesForEntity(Entity aEntity, bool aIsFirst = true);

	protected:
		Entity myEntity;
		bool myIsStatic = true;
		float myMass = 0.0f;
		std::unordered_map<ShapeType, std::vector<std::shared_ptr<PhysicsShape>>> myShapes;
	};
}
