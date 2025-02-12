#include "epch.h"
#include "Entity.h"
#include "Scene.h"

namespace Epoch
{
	bool Entity::IsActive(bool aCheckAncestor) const
	{
		if (!GetComponent<ActiveComponent>().isActive)
		{
			return false;
		}

		if (aCheckAncestor)
		{
			return IsAncestorActive();
		}

		return true;
	}

	bool Entity::IsAncestorActive() const
	{
		Entity parent = GetParent();
		
		if (!parent)
		{
			return true;
		}

		if (!parent.IsActive())
		{
			return false;
		}

		return parent.IsAncestorActive();
	}

	Entity Entity::GetParent() const
	{
		return myScene->TryGetEntityWithUUID(GetParentUUID());
	}

	void Entity::SetParent(Entity aParent)
	{
		Entity currentParent = GetParent();
		if (currentParent == aParent)
		{
			return;
		}

		// If changing parent, remove child from existing parent
		if (currentParent)
		{
			currentParent.RemoveChild(*this);
		}

		// Setting to null is okay
		SetParentUUID(aParent.GetUUID());

		if (aParent)
		{
			auto& parentChildren = aParent.Children();
			UUID uuid = GetUUID();
			if (std::find(parentChildren.begin(), parentChildren.end(), uuid) == parentChildren.end())
			{
				parentChildren.emplace_back(GetUUID());
			}
		}
	}

	std::vector<Entity> Entity::GetChildren()
	{
		std::vector<Entity> entities;
		entities.reserve(Children().size());

		for (auto id : Children())
		{
			entities.push_back(myScene->GetEntityWithUUID(id));
		}

		return entities;
	}

	bool Entity::RemoveChild(Entity aChild)
	{
		UUID childId = aChild.GetUUID();
		std::vector<UUID>& children = Children();
		auto it = std::find(children.begin(), children.end(), childId);
		if (it != children.end())
		{
			children.erase(it);
			return true;
		}

		return false;
	}

	bool Entity::IsAncestorOf(Entity aEntity) const
	{
		const auto& children = Children();

		if (children.empty())
		{
			return false;
		}

		for (UUID child : children)
		{
			if (child == aEntity.GetUUID())
			{
				return true;
			}
		}

		for (UUID child : children)
		{
			if (myScene->GetEntityWithUUID(child).IsAncestorOf(aEntity))
			{
				return true;
			}
		}

		return false;
	}

	CU::Transform Entity::GetWorldSpaceTransform()
	{
		return myScene->GetWorldSpaceTransform(*this);
	}
}
