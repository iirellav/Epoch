#pragma once
#include <unordered_map>
#include "PhysicsEventCallback.h"
#include "PhysicsBody.h"
#include "CharacterController.h"
#include "SceneQueries.h"
#include "PhysicsLayer.h"

namespace Epoch
{
	class Scene;

	class PhysicsScene
	{
	public:
		PhysicsScene(Scene* aScene);
		virtual ~PhysicsScene();

		virtual void Destroy() = 0;
		virtual void Simulate() = 0;

		virtual std::shared_ptr<PhysicsBody> CreateBody(Entity aEntity) = 0;
		virtual void DestroyBody(Entity aEntity) = 0;
		virtual std::shared_ptr<CharacterController> CreateCharacterController(Entity aEntity) = 0;
		
		std::shared_ptr<PhysicsBody> GetPhysicsBodyWithID(UUID aID);
		std::shared_ptr<PhysicsBody> GetPhysicsBody(Entity aEntity);
		std::shared_ptr<CharacterController> GetCharacterControllerWithID(UUID aID);
		std::shared_ptr<CharacterController> GetCharacterController(Entity aEntity);

		uint32_t GetStaticPhysicsBodyCount() const { return (uint32_t)myStaticPhysicsBodies.size(); }
		uint32_t GetDynamicPhysicsBodyCount() const { return (uint32_t)myDynamicPhysicsBodies.size(); }
		uint32_t GetCharacterControllerCount() const { return (uint32_t)myCharacterControllers.size(); }

		virtual CU::Vector3f GetGravity() const = 0;
		virtual void SetGravity(const CU::Vector3f& aGravity) = 0;

		virtual bool Raycast(CU::Vector3f aOrigin, CU::Vector3f aDirection, float aMaxDistance = FLT_MAX, HitInfo* outHit = nullptr, LayerMask* aLayerMask = nullptr) = 0;
		virtual bool ShapeCast(const ShapeCastInfo* aShapeCastInfo, HitInfo* outHit) = 0;
		virtual std::vector<UUID> OverlapShape(const ShapeOverlapInfo* aShapeOverlapInfo) = 0;

		void AddRadialImpulse(CU::Vector3f aOrigin, float aRadius, float aStrength);
		
		virtual void Teleport(Entity aEntity, const CU::Vector3f& aTargetPosition, const CU::Quatf& aTargetRotation) = 0;

	protected:
		void CreatePhysicsBodies();
		void CreateCharacterControllers();
		
		void SubStepStrategy();
		void PreSimulate();
		void PostSimulate();

		void OnPhysicsEvent(PhysicsEventType aType, Entity aEntityA, Entity aEntityB);
		
	protected:
		Scene* mySceneContext;

		std::unordered_map<UUID, std::shared_ptr<PhysicsBody>> myStaticPhysicsBodies;
		std::unordered_map<UUID, std::shared_ptr<PhysicsBody>> myDynamicPhysicsBodies;
		std::unordered_map<UUID, std::shared_ptr<CharacterController>> myCharacterControllers;

		float myAccumulator = 0.0f;
		uint32_t mySubSteps = 1;

	private:
		struct PhysicsEvent { PhysicsEventType type = PhysicsEventType::None; UUID entityA; UUID entityB; };
		std::vector<PhysicsEvent> myPhysicsEvents;
	};
}
