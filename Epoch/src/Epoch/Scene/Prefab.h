#pragma once
#include <memory>
#include <unordered_set>
#include "Epoch/Assets/Asset.h"
#include "Epoch/Scene/Scene.h"
#include "Epoch/Scene/Entity.h"

namespace Epoch
{
	class Prefab : public Asset
	{
	public:
		Prefab();
		~Prefab() = default;
		
		void Create(Entity aEntity);

		std::unordered_set<AssetHandle> GetAssetList();

		static AssetType GetStaticType() { return AssetType::Prefab; }
		virtual AssetType GetAssetType() const override { return GetStaticType(); }
		
	private:
		Entity CreatePrefabFromEntity(Entity aEntity);

	private:
		std::shared_ptr<Scene> myScene;
		Entity myEntity;

		friend class Scene;
		friend class PrefabSerializer;
		friend class InspectorPanel;
	};
}
