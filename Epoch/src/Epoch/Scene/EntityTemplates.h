#pragma once

namespace Epoch
{
	template<typename T, typename... Args>
		T& Entity::AddComponent(Args&&... args)
		{
			EPOCH_ASSERT(!HasComponent<T>(), "Entity already has component!");
			T& component = myScene->myRegistry.emplace<T>(myEntityHandle, std::forward<Args>(args)...);
			return component;
		}

		template<typename T, typename... Args>
		T& Entity::AddOrReplaceComponent(Args&&... args)
		{
			T& component = myScene->myRegistry.emplace_or_replace<T>(myEntityHandle, std::forward<Args>(args)...);
			return component;
		}

		template<typename T>
		bool Entity::HasComponent()
		{
			return myScene->myRegistry.has<T>(myEntityHandle);
		}

		template<typename T>
		bool Entity::HasComponent() const
		{
			return myScene->myRegistry.has<T>(myEntityHandle);
		}

		template<typename...T>
		bool Entity::HasAny()
		{
			return myScene->myRegistry.any<T...>(myEntityHandle);
		}

		template<typename...T>
		bool Entity::HasAny() const
		{
			return myScene->myRegistry.any<T...>(myEntityHandle);
		}

		template<typename T>
		T& Entity::GetComponent()
		{
			EPOCH_ASSERT(HasComponent<T>(), "Entity does not have component!");
			return myScene->myRegistry.get<T>(myEntityHandle);
		}

		template<typename T>
		const T& Entity::GetComponent() const
		{
			EPOCH_ASSERT(HasComponent<T>(), "Entity does not have component!");
			return myScene->myRegistry.get<T>(myEntityHandle);
		}

		template<typename T>
		void Entity::RemoveComponent()
		{
			EPOCH_ASSERT(HasComponent<T>(), "Entity does not have component!");
			myScene->myRegistry.remove<T>(myEntityHandle);
		}
}
