#pragma once
#include <entt/entt.hpp>
#include "Epoch/Debug/Log.h"
#include "Epoch/Core/UUID.h"
#include "Components.h"

namespace Epoch
{
	class Scene;

	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity aHandle, Scene* aScene) : myEntityHandle(aHandle), myScene(aScene) {}
		Entity(const Entity&) = default;
		~Entity() = default;

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args);

		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Args&&... args);

		template<typename T>
		bool HasComponent();

		template<typename T>
		bool HasComponent() const;

		template<typename...T>
		bool HasAny();
		
		template<typename...T>
		bool HasAny() const;

		template<typename T>
		T& GetComponent();

		template<typename T>
		const T& GetComponent() const;

		template<typename T>
		void RemoveComponent();

		UUID GetUUID() { return GetComponent<IDComponent>().id; }
		const std::string& GetName() { return GetComponent<NameComponent>().name; }

		bool IsActive(bool aCheckAncestor = true) const;
		void SetIsActive(bool aState) { GetComponent<ActiveComponent>().isActive = aState; }
		bool IsAncestorActive() const;

		bool HasParent() const { return GetComponent<RelationshipComponent>().parentHandle != 0; }
		Entity GetParent() const;
		void SetParent(Entity aParent);

		void SetParentUUID(UUID aParent) { GetComponent<RelationshipComponent>().parentHandle = aParent; }
		UUID GetParentUUID() const { return GetComponent<RelationshipComponent>().parentHandle; }

		std::vector<UUID>& Children() { return GetComponent<RelationshipComponent>().children; }
		const std::vector<UUID>& Children() const { return GetComponent<RelationshipComponent>().children; }
		std::vector<Entity> GetChildren();
		bool RemoveChild(Entity aChild);

		bool IsAncestorOf(Entity aEntity) const;
		bool IsDescendantOf(Entity aEntity) const { return aEntity.IsAncestorOf(*this); }

		CU::Transform& Transform() { return GetComponent<TransformComponent>().transform; }
		const CU::Matrix4x4f& TransformMatrix() { return GetComponent<TransformComponent>().GetMatrix(); }

		CU::Transform GetWorldSpaceTransform();

		operator bool() const { return myEntityHandle != entt::null; }
		operator entt::entity() const { return myEntityHandle; }
		operator uint32_t() const { return (uint32_t)myEntityHandle; }

		bool operator==(const Entity& aOther) const
		{
			return myEntityHandle == aOther.myEntityHandle && myScene == aOther.myScene;
		}

		bool operator!=(const Entity& aOther) const
		{
			return !(*this == aOther);
		}

	private:
		entt::entity myEntityHandle{ entt::null };
		Scene* myScene = nullptr;

		friend class Scene;
		friend class Prefab;
	};
}
