#pragma once
#include <string>
#include <memory>
#include <unordered_set>
#include <unordered_map>

#include "Entity.h"
#include "SceneInfo.h"
#include "Epoch/Core/UUID.h"
#include "Epoch/Assets/Asset.h"
#include "Epoch/Math/Frustum.h"
#include "Epoch/Editor/EditorCamera.h"
#include "Epoch/Physics/PhysicsSystem.h"

#include "Epoch/Rendering/Mesh.h" //TODO: Fix
#include "Epoch/Animation/Skeleton.h" //TODO: Fix

namespace Epoch
{
	class Prefab;
	class SceneRenderer;
	struct SceneRendererCamera;

	class Scene : public Asset
	{
	public:
		struct PerformanceTimers
		{
			float scriptUpdate = 0.0f;
			float scriptLateUpdate = 0.0f;
			float scriptFixedUpdate = 0.0f;
			float physicsSimulation = 0.0f;
		};

	public:
		Scene(const std::string& aName = "New Scene") : myName(aName) {}
		Scene(AssetHandle aHandle) { SetAssetHandle(aHandle); }
		~Scene() = default;
		
		void SetSceneTransitionCallback(const std::function<void(AssetHandle)>& aCallback) { myOnSceneTransitionCallback = aCallback; }
		void SetEntityDestroyedCallback(const std::function<void(Entity)>& aCallback) { myOnEntityDestroyedCallback = aCallback; }

		static AssetType GetStaticType() { return AssetType::Scene; }
		AssetType GetAssetType() const override { return GetStaticType(); }

		void CopyTo(std::shared_ptr<Scene> aCopy);
		void SortEntities();

		Entity CreateEntity(const std::string& aName = "New Entity");
		Entity CreateChildEntity(Entity aParent, const std::string& aName = "New Entity");
		Entity CreateEntityWithUUID(UUID aUUID, const std::string& aName = "New Entity");

		void DestroyEntity(Entity aEntity, bool aFirst = true);
		void SubmitToDestroyEntity(Entity aEntity);

		Entity DuplicateEntity(Entity aEntity, bool aFirst = true);

		Entity Instantiate(std::shared_ptr<Prefab> aPrefab, const CU::Vector3f* aTranslation = nullptr, const CU::Vector3f* aRotation = nullptr, const CU::Vector3f* aScale = nullptr);
		Entity InstantiateChild(std::shared_ptr<Prefab> aPrefab, Entity aParent, const CU::Vector3f* aTranslation = nullptr, const CU::Vector3f* aRotation = nullptr, const CU::Vector3f* aScale = nullptr);
		Entity CreatePrefabEntity(Entity aEntity, Entity aParent);
		Entity InstantiateMesh(std::shared_ptr<Mesh> aMesh);

		void UpdateEntityReferences(const std::unordered_map<UUID, UUID>& aEntityIDMap);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnSimulationStart();
		void OnSimulationStop();

		void OnUpdateRuntime();
		void OnUpdateEditor();

		void OnRenderGame(std::shared_ptr<SceneRenderer> aRenderer);

		struct EditorRenderSettings
		{
			bool cullWithGameCamera = false;
			bool postProcessingEnabled = false;
			bool displayUI = false;
		};
		void OnRenderEditor(std::shared_ptr<SceneRenderer> aRenderer, EditorCamera& aCamera, const EditorRenderSettings& aSettings);

		void OnSceneTransition(AssetHandle aScene);

		void SetMousePos(const CU::Vector2f& aPos) { myMousePos = aPos; }
		void SetViewportSize(uint32_t aWidth, uint32_t aHeight) { myViewportWidth = aWidth; myViewportHeight = aHeight; }
		unsigned GetViewportWidth() const { return myViewportWidth; }
		unsigned GetViewportHeight() const { return myViewportHeight; }

		Entity TryGetEntityByName(std::string_view aName);
		std::vector<Entity> GetEntitiesByName(std::string_view aName);
		Entity GetEntityWithUUID(UUID aUUID);
		Entity TryGetEntityWithUUID(UUID aUUID);
		Entity TryGetDescendantEntityWithName(Entity aEntity, const std::string& aName);
		bool IsEntityValid(Entity aEntity) const;
		bool WasEntityFrustumCulled(UUID aEntityID) const { return myFrustumCulledEntities.find(aEntityID) != myFrustumCulledEntities.end(); }

		Entity GetPrimaryCameraEntity();

		std::shared_ptr<PhysicsScene> GetPhysicsScene() { return myPhysicsScene; }
		std::pair<uint32_t, uint32_t> GetPhysicsBodyCount();

		template<typename... ComponentTypse>
		auto GetAllEntitiesWith()
		{
			return myRegistry.view<ComponentTypse...>();
		}

		CU::Matrix4x4f GetWorldSpaceTransformMatrix(Entity aEntity);
		CU::Transform GetWorldSpaceTransform(Entity aEntity);

		void ConvertToLocalSpace(Entity aEntity);
		void ConvertToWorldSpace(Entity aEntity);

		void ParentEntity(Entity aEntity, Entity aParent);
		void UnparentEntity(Entity aEntity, bool aConvertToWorldSpace = true);

		bool IsSimulating() const { return myIsSimulating; }
		bool IsPlaying() const { return myIsPlaying; }
		bool IsPaused() const { return myIsPaused; }

		void SetPaused(bool aPaused) { myIsPaused = aPaused; }
		void Step(unsigned aFrames = 1) { myStepFrames = aFrames; }

		float GetTimeScale() const { return myTimeScale; }
		void SetTimeScale(float aTimeScale) { myTimeScale = aTimeScale; }

		const std::string& GetName() { return myName; }

		std::unordered_set<AssetHandle> GetAllSceneReferences();
		std::unordered_set<AssetHandle> GetAllSceneAssets();

		PerformanceTimers& GetPerformanceTimers() { return myPerformanceTimers; }
		const PerformanceTimers& GetPerformanceTimers() const { return myPerformanceTimers; }

		template<typename Fn>
		void SubmitPostUpdateFunc(Fn&& aFunc)
		{
			myPostUpdateQueue.emplace_back(aFunc);
		}


		template<typename... Component>
		static void CopyComponent(entt::registry& aDst, entt::registry& aSrc, const std::unordered_map<UUID, entt::entity>& aEnttMap)
		{
			([&]()
				{
					auto view = aSrc.view<Component>();
					for (auto srcEntity : view)
					{
						entt::entity dstEntity = aEnttMap.at(aSrc.get<IDComponent>(srcEntity).id);

						auto& srcComponent = aSrc.get<Component>(srcEntity);
						aDst.emplace_or_replace<Component>(dstEntity, srcComponent);
					}
				}(), ...);
		}

		template<typename... Component>
		static void CopyComponent(ComponentGroup<Component...>, entt::registry& aDst, entt::registry& aSrc, const std::unordered_map<UUID, entt::entity>& aEnttMap)
		{
			CopyComponent<Component...>(aDst, aSrc, aEnttMap);
		}

		template<typename... Component>
		static void CopyComponentIfExists(Entity aDst, Entity aSrc)
		{
			([&]()
				{
					if (aSrc.HasComponent<Component>())
						aDst.AddOrReplaceComponent<Component>(aSrc.GetComponent<Component>());
				}(), ...);
		}

		template<typename... Component>
		static void CopyComponentsIfExists(ComponentGroup<Component...>, Entity aDst, Entity aSrc)
		{
			CopyComponentIfExists<Component...>(aDst, aSrc);
		}


		template<typename Component>
		static void CopyComponentFromScene(std::shared_ptr<Scene> aSrcScene, Entity aSrc, std::shared_ptr<Scene> aDstScene, Entity aDst)
		{
			if (aSrcScene->myRegistry.has<Component>(aSrc))
			{
				auto& srcComponent = aSrcScene->myRegistry.get<Component>(aSrc);
				aDstScene->myRegistry.emplace_or_replace<Component>(aDst, srcComponent);
			}
		}

	private:
		bool ShouldStep()
		{
			if (myStepFrames > 0)
			{
				myStepFrames--;
				return true;
			}
			return false;
		}

		void BuildMeshEntityHierarchy(Entity aParent, std::shared_ptr<Mesh> aMesh, const MeshNode& aNode);
		void FindBoneEntityIds(Entity aRoot);
		std::vector<UUID> FindBoneEntityIds(Entity aRoot, std::shared_ptr<Mesh> aMesh);

		std::vector<CU::Matrix4x4f> GetModelSpaceBoneTransforms(Entity aEntity, std::shared_ptr<Mesh> aMesh);
		void GetModelSpaceBoneTransform(const std::vector<UUID>& aBoneEntityIds, std::vector<CU::Matrix4x4f>& outBoneTransforms, uint32_t aBoneIndex, const CU::Matrix4x4f& aParentTransform, std::shared_ptr<Skeleton> aSkeleton);

		void Render3DScene(std::shared_ptr<SceneRenderer> aRenderer, const SceneRendererCamera& aRenderCamera, const SceneRendererCamera& aCullingCamera, bool aIsGameView, bool aWithPostProccessing = true);
		void Render2DScene(std::shared_ptr<SceneRenderer> aRenderer, const SceneRendererCamera& aRenderCamera, bool aIsGameView);
		Frustum CreateFrustum(const SceneRendererCamera& aCamera);
		bool FrustumIntersection(const Frustum& aFrustum, const AABB aAABB);
		
		void SetName(const std::string& aName) { myName = aName; }
		void SetAssetHandle(AssetHandle aHandle) { myHandle = aHandle; }

		std::pair<float, float> GetMouseViewportSpace() const;
		bool MouseInViewport();

	private:
		entt::registry myRegistry;
		std::unordered_map<UUID, entt::entity> myEntityMap;

		//This gets filled in Scene::RenderScene, so if this is used before Scene::RenderScene has been called the set contains the last frames culled entities.
		std::unordered_set<UUID> myFrustumCulledEntities;

		std::shared_ptr<PhysicsScene> myPhysicsScene;

		std::vector<std::function<void()>> myPostUpdateQueue;
		
		std::function<void(AssetHandle)> myOnSceneTransitionCallback;
		std::function<void(Entity)> myOnEntityDestroyedCallback;

		CU::Vector2f myMousePos;
		uint32_t myViewportWidth = 0;
		uint32_t myViewportHeight = 0;

		LightEnvironment myLightEnvironment;
		PostProcessingData myPostProcessingData;

		bool myIsSimulating = false;
		bool myIsPlaying = false;
		bool myIsPaused = false;
		unsigned myStepFrames = 0;

		float myTimeScale = 1.0f;

		std::string myName = "New Scene";
		PerformanceTimers myPerformanceTimers;

		friend class EditorLayer;
		friend class Entity;
		friend class SceneSerializer;
		friend class PrefabSerializer;
		friend class SceneRenderer;
		friend class SceneHierarchyPanel;
		friend class AssetPack;
	};
}

#include "EntityTemplates.h"
