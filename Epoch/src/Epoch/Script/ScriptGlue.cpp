#include "epch.h"
#include "ScriptGlue.h"
#include <functional>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <mono/metadata/class.h>
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>
#include <mono/metadata/loader.h>
#include <mono/metadata/exception.h>
#include <mono/jit/jit.h>

#include <CommonUtilities/Timer.h>
#include "Epoch/Utils/TypeInfo.h"
#include "ScriptEngine.h"
#include "ScriptUtils.h"
#include "ScriptCache.h"
#include "CSharpInstanceInspector.h"
#include "Epoch/Assets/AssetManager.h"
#include "Epoch/Scene/Scene.h"
#include "Epoch/Core/Input.h"
#include "Epoch/Core/Application.h"
#include "Epoch/Core/GraphicsEngine.h"
#include "Epoch/Scene/SceneRenderer.h"
#include "Epoch/Rendering/DebugRenderer.h"

namespace Epoch
{
#define EPOCH_ADD_INTERNAL_CALL(iCall) mono_add_internal_call("Epoch.InternalCalls::"#iCall, (void*)InternalCalls::iCall)

	std::unordered_map<MonoType*, std::function<void(Entity&)>> staticAddComponentFuncs;
	std::unordered_map<MonoType*, std::function<bool(Entity&)>> staticHasComponentFuncs;
	std::unordered_map<MonoType*, std::function<void(Entity&)>> staticRemoveComponentFuncs;

	template<typename TComponent>
	static void RegisterManagedComponent()
	{
		const TypeNameString& componentTypeName = TypeInfo<TComponent, true>().Name();
		std::string componentName = fmt::format("Epoch.{}", componentTypeName);

		MonoType* managedType = mono_reflection_type_from_name(componentName.data(), ScriptEngine::GetCoreAssemblyInfo()->assemblyImage);

		if (managedType)
		{
			staticAddComponentFuncs[managedType] = [](Entity& aEntity) { aEntity.AddComponent<TComponent>(); };
			staticHasComponentFuncs[managedType] = [](Entity& aEntity) { return aEntity.HasComponent<TComponent>(); };
			staticRemoveComponentFuncs[managedType] = [](Entity& aEntity) { aEntity.RemoveComponent<TComponent>(); };
		}
		else
		{
			EPOCH_ASSERT(false, std::format("No C# component class found for {}!", componentName));
		}
	}

	void ScriptGlue::RegisterGlue()
	{
		EPOCH_PROFILE_FUNC();

		if (staticAddComponentFuncs.size() > 0)
		{
			staticAddComponentFuncs.clear();
			staticHasComponentFuncs.clear();
			staticRemoveComponentFuncs.clear();
		}
		
		RegisterComponentTypes();
		RegisterInternalCalls();
	}

	void ScriptGlue::RegisterComponentTypes()
	{
		RegisterManagedComponent<NameComponent>();
		RegisterManagedComponent<TransformComponent>();
		RegisterManagedComponent<ScriptComponent>();
		RegisterManagedComponent<TextRendererComponent>();
		RegisterManagedComponent<PointLightComponent>();
		RegisterManagedComponent<SpotlightComponent>();
		RegisterManagedComponent<RigidbodyComponent>();
		RegisterManagedComponent<CharacterControllerComponent>();
	}

	void ScriptGlue::RegisterInternalCalls()
	{
		EPOCH_ADD_INTERNAL_CALL(Application_Quit);

		EPOCH_ADD_INTERNAL_CALL(Application_GetIsVSync);
		EPOCH_ADD_INTERNAL_CALL(Application_SetIsVSync);

		EPOCH_ADD_INTERNAL_CALL(Application_GetWidth);
		EPOCH_ADD_INTERNAL_CALL(Application_GetHeight);


		EPOCH_ADD_INTERNAL_CALL(Time_GetTimeScale);
		EPOCH_ADD_INTERNAL_CALL(Time_SetTimeScale);


		EPOCH_ADD_INTERNAL_CALL(AssetHandle_IsValid);


		EPOCH_ADD_INTERNAL_CALL(SceneManager_LoadScene);
		EPOCH_ADD_INTERNAL_CALL(SceneManager_GetCurrentSceneName);


		EPOCH_ADD_INTERNAL_CALL(Scene_IsEntityValid);

		EPOCH_ADD_INTERNAL_CALL(Scene_GetEntityByName);

		EPOCH_ADD_INTERNAL_CALL(Scene_CreateEntity);
		EPOCH_ADD_INTERNAL_CALL(Scene_DestroyEntity);
		EPOCH_ADD_INTERNAL_CALL(Scene_DestroyAllChildren);

		EPOCH_ADD_INTERNAL_CALL(Scene_InstantiatePrefab);
		EPOCH_ADD_INTERNAL_CALL(Scene_InstantiatePrefabWithTranslation);
		EPOCH_ADD_INTERNAL_CALL(Scene_InstantiatePrefabWithTranslationAndRotation);
		EPOCH_ADD_INTERNAL_CALL(Scene_InstantiatePrefabWithTransform);

		EPOCH_ADD_INTERNAL_CALL(Scene_InstantiatePrefabWithParent);
		EPOCH_ADD_INTERNAL_CALL(Scene_InstantiatePrefabWithTranslationWithParent);
		EPOCH_ADD_INTERNAL_CALL(Scene_InstantiatePrefabWithTranslationAndRotationWithParent);
		EPOCH_ADD_INTERNAL_CALL(Scene_InstantiatePrefabWithTransformWithParent);

		
		EPOCH_ADD_INTERNAL_CALL(Entity_GetIsActive);
		EPOCH_ADD_INTERNAL_CALL(Entity_SetIsActive);

		EPOCH_ADD_INTERNAL_CALL(Entity_GetParent);
		EPOCH_ADD_INTERNAL_CALL(Entity_SetParent);
		EPOCH_ADD_INTERNAL_CALL(Entity_GetChildren);
		EPOCH_ADD_INTERNAL_CALL(Entity_GetChildByName);

		EPOCH_ADD_INTERNAL_CALL(Entity_HasComponent);
		EPOCH_ADD_INTERNAL_CALL(Entity_RemoveComponent);


		EPOCH_ADD_INTERNAL_CALL(NameComponent_GetName);
		EPOCH_ADD_INTERNAL_CALL(NameComponent_SetName);


		EPOCH_ADD_INTERNAL_CALL(TransformComponent_GetTransform);
		EPOCH_ADD_INTERNAL_CALL(TransformComponent_SetTransform);
		EPOCH_ADD_INTERNAL_CALL(TransformComponent_GetWorldSpaceTransform);
		EPOCH_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		EPOCH_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
		EPOCH_ADD_INTERNAL_CALL(TransformComponent_GetScale);
		EPOCH_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
		EPOCH_ADD_INTERNAL_CALL(TransformComponent_SetRotation);
		EPOCH_ADD_INTERNAL_CALL(TransformComponent_SetScale);
		EPOCH_ADD_INTERNAL_CALL(TransformComponent_Translate);
		EPOCH_ADD_INTERNAL_CALL(TransformComponent_Rotate);
		EPOCH_ADD_INTERNAL_CALL(TransformComponent_RotateAround);
		EPOCH_ADD_INTERNAL_CALL(TransformComponent_LookAt);


		EPOCH_ADD_INTERNAL_CALL(ScriptComponent_GetInstance);


		EPOCH_ADD_INTERNAL_CALL(TextComponent_GetText);
		EPOCH_ADD_INTERNAL_CALL(TextComponent_SetText);
		EPOCH_ADD_INTERNAL_CALL(TextComponent_GetColor);
		EPOCH_ADD_INTERNAL_CALL(TextComponent_SetColor);


		EPOCH_ADD_INTERNAL_CALL(PointLightComponent_GetColor);
		EPOCH_ADD_INTERNAL_CALL(PointLightComponent_SetColor);

		EPOCH_ADD_INTERNAL_CALL(PointLightComponent_GetIntensity);
		EPOCH_ADD_INTERNAL_CALL(PointLightComponent_SetIntensity);

		EPOCH_ADD_INTERNAL_CALL(PointLightComponent_GetCastsShadows);
		EPOCH_ADD_INTERNAL_CALL(PointLightComponent_SetCastsShadows);

		EPOCH_ADD_INTERNAL_CALL(PointLightComponent_GetRange);
		EPOCH_ADD_INTERNAL_CALL(PointLightComponent_SetRange);


		EPOCH_ADD_INTERNAL_CALL(SpotlightComponent_GetColor);
		EPOCH_ADD_INTERNAL_CALL(SpotlightComponent_SetColor);

		EPOCH_ADD_INTERNAL_CALL(SpotlightComponent_GetIntensity);
		EPOCH_ADD_INTERNAL_CALL(SpotlightComponent_SetIntensity);

		EPOCH_ADD_INTERNAL_CALL(SpotlightComponent_GetCastsShadows);
		EPOCH_ADD_INTERNAL_CALL(SpotlightComponent_SetCastsShadows);

		EPOCH_ADD_INTERNAL_CALL(SpotlightComponent_GetRange);
		EPOCH_ADD_INTERNAL_CALL(SpotlightComponent_SetRange);


		EPOCH_ADD_INTERNAL_CALL(Log_LogMessage);


		EPOCH_ADD_INTERNAL_CALL(Gizmos_DrawWireSphere);
		EPOCH_ADD_INTERNAL_CALL(Gizmos_DrawWireCube);
		EPOCH_ADD_INTERNAL_CALL(Gizmos_DrawLine);


		EPOCH_ADD_INTERNAL_CALL(Input_IsKeyPressed);
		EPOCH_ADD_INTERNAL_CALL(Input_IsKeyHeld);
		EPOCH_ADD_INTERNAL_CALL(Input_IsKeyReleased);
		
		EPOCH_ADD_INTERNAL_CALL(Input_IsMouseButtonPressed);
		EPOCH_ADD_INTERNAL_CALL(Input_IsMouseButtonHeld);
		EPOCH_ADD_INTERNAL_CALL(Input_IsMouseButtonReleased);

		EPOCH_ADD_INTERNAL_CALL(Input_GetMouseDelta);

		EPOCH_ADD_INTERNAL_CALL(Input_GetCursorMode);
		EPOCH_ADD_INTERNAL_CALL(Input_SetCursorMode);

		EPOCH_ADD_INTERNAL_CALL(Input_GetScrollDelta);

		EPOCH_ADD_INTERNAL_CALL(Input_IsGamepadButtonPressed);
		EPOCH_ADD_INTERNAL_CALL(Input_IsGamepadButtonHeld);
		EPOCH_ADD_INTERNAL_CALL(Input_IsGamepadButtonReleased);

		EPOCH_ADD_INTERNAL_CALL(Input_GetGamepadAxis);


		EPOCH_ADD_INTERNAL_CALL(Physics_Raycast);

		EPOCH_ADD_INTERNAL_CALL(Physics_GetGravity);
		EPOCH_ADD_INTERNAL_CALL(Physics_SetGravity);
		
		EPOCH_ADD_INTERNAL_CALL(Physics_AddRadialImpulse);


		EPOCH_ADD_INTERNAL_CALL(RigidbodyComponent_GetMass);
		EPOCH_ADD_INTERNAL_CALL(RigidbodyComponent_SetMass);

		EPOCH_ADD_INTERNAL_CALL(RigidbodyComponent_GetUseGravity);
		EPOCH_ADD_INTERNAL_CALL(RigidbodyComponent_SetUseGravity);

		EPOCH_ADD_INTERNAL_CALL(RigidbodyComponent_GetVelocity);
		EPOCH_ADD_INTERNAL_CALL(RigidbodyComponent_SetVelocity);

		EPOCH_ADD_INTERNAL_CALL(RigidbodyComponent_GetAngularVelocity);
		EPOCH_ADD_INTERNAL_CALL(RigidbodyComponent_SetAngularVelocity);

		EPOCH_ADD_INTERNAL_CALL(RigidbodyComponent_GetPosition);
		EPOCH_ADD_INTERNAL_CALL(RigidbodyComponent_SetPosition);

		EPOCH_ADD_INTERNAL_CALL(RigidbodyComponent_GetRotation);
		EPOCH_ADD_INTERNAL_CALL(RigidbodyComponent_SetRotation);

		EPOCH_ADD_INTERNAL_CALL(RigidbodyComponent_GetConstraints);
		EPOCH_ADD_INTERNAL_CALL(RigidbodyComponent_SetConstraints);

		EPOCH_ADD_INTERNAL_CALL(RigidbodyComponent_AddForce);
		EPOCH_ADD_INTERNAL_CALL(RigidbodyComponent_AddForceAtPosition);

		EPOCH_ADD_INTERNAL_CALL(RigidbodyComponent_AddTorque);

		EPOCH_ADD_INTERNAL_CALL(RigidbodyComponent_Teleport);


		EPOCH_ADD_INTERNAL_CALL(CharacterControllerComponent_GetStepOffset);
		EPOCH_ADD_INTERNAL_CALL(CharacterControllerComponent_SetStepOffset);
		
		EPOCH_ADD_INTERNAL_CALL(CharacterControllerComponent_GetSlopeLimit);
		EPOCH_ADD_INTERNAL_CALL(CharacterControllerComponent_SetSlopeLimit);

		EPOCH_ADD_INTERNAL_CALL(CharacterControllerComponent_Resize);

		EPOCH_ADD_INTERNAL_CALL(CharacterControllerComponent_Move);
	}

	namespace InternalCalls
	{
		static inline Entity GetEntity(uint64_t aEntityID)
		{
			std::shared_ptr<Scene> scene = ScriptEngine::GetSceneContext();
			EPOCH_ASSERT(scene, "No active scene!");
			return scene->TryGetEntityWithUUID(aEntityID);
		};
		
		std::shared_ptr<PhysicsBody> GetPhysicsBody(uint64_t entityID)
		{
			std::shared_ptr<Scene> scene = ScriptEngine::GetSceneContext();

			if (!scene->IsPlaying()) return nullptr;

			Entity entity = scene->TryGetEntityWithUUID(entityID);
			if (!entity) return nullptr;

			return scene->GetPhysicsScene()->GetPhysicsBody(entity);
		}

		std::shared_ptr<CharacterController> GetCharacterController(uint64_t entityID)
		{
			std::shared_ptr<Scene> scene = ScriptEngine::GetSceneContext();

			if (!scene->IsPlaying()) return nullptr;

			Entity entity = scene->TryGetEntityWithUUID(entityID);
			if (!entity) return nullptr;

			return scene->GetPhysicsScene()->GetCharacterController(entity);
		}

#pragma region Application

		void Application_Quit()
		{
			if (Application::IsRuntime())
			{
				Application::Get().Close();
			}
		}

		bool Application_GetIsVSync()
		{
			return GraphicsEngine::Get().GetVSyncBool();
		}

		void Application_SetIsVSync(bool aState)
		{
			GraphicsEngine::Get().GetVSyncBool() = aState;
		}

		uint32_t Application_GetWidth()
		{
			return ScriptEngine::GetSceneContext()->GetViewportWidth();
		}

		uint32_t Application_GetHeight()
		{
			return ScriptEngine::GetSceneContext()->GetViewportHeight();
		}

#pragma endregion

#pragma region Time
		
		float Time_GetTimeScale()
		{
			return ScriptEngine::GetSceneContext()->GetTimeScale();
		}

		void Time_SetTimeScale(float aTimeScale)
		{
			ScriptEngine::GetSceneContext()->SetTimeScale(aTimeScale);
		}

#pragma endregion

#pragma region AssetHandle

		bool AssetHandle_IsValid(AssetHandle* aAssetHandle)
		{
			return AssetManager::IsAssetHandleValid(*aAssetHandle);
		}

#pragma endregion

#pragma region SceneManager

		void SceneManager_LoadScene(AssetHandle* aSceneHandle)
		{
			std::shared_ptr<Scene> scene = ScriptEngine::GetSceneContext();
			EPOCH_ASSERT(scene, "No active scene!");

			if (!AssetManager::IsAssetHandleValid(*aSceneHandle))
			{
				return;
			}

			scene->OnSceneTransition(*aSceneHandle);
		}

		MonoString* SceneManager_GetCurrentSceneName()
		{
			std::shared_ptr<Scene> scene = ScriptEngine::GetSceneContext();
			EPOCH_ASSERT(scene, "No active scene!");
			return ScriptUtils::UTF8StringToMono(scene->GetName());
		}

#pragma endregion

#pragma region Scene

		bool Scene_IsEntityValid(uint64_t aEntityID)
		{
			return (bool)GetEntity(aEntityID);
		}

		uint64_t Scene_GetEntityByName(MonoString* aName)
		{
			std::shared_ptr<Scene> scene = ScriptEngine::GetSceneContext();
			EPOCH_ASSERT(scene, "No active scene!");
			Entity entity = scene->TryGetEntityByName(ScriptUtils::MonoStringToUTF8(aName));
			return entity ? entity.GetUUID() : UUID(0);
		}

		uint64_t Scene_CreateEntity(MonoString* aName)
		{
			std::shared_ptr<Scene> scene = ScriptEngine::GetSceneContext();
			EPOCH_ASSERT(scene, "No active scene!");
			return scene->CreateEntity(ScriptUtils::MonoStringToUTF8(aName)).GetUUID();
		}

		void Scene_DestroyEntity(uint64_t aEntityID)
		{
			std::shared_ptr<Scene> scene = ScriptEngine::GetSceneContext();
			EPOCH_ASSERT(scene, "No active scene!");
			Entity entity = GetEntity(aEntityID);
			if (!entity) return;
			scene->SubmitToDestroyEntity(entity);
		}

		void Scene_DestroyAllChildren(uint64_t aEntityID)
		{
			std::shared_ptr<Scene> scene = ScriptEngine::GetSceneContext();
			EPOCH_ASSERT(scene, "No active scene!");
			Entity entity = GetEntity(aEntityID);
			if (!entity) return;

			const std::vector<UUID> children = entity.Children();
			for (UUID id : children)
			{
				scene->SubmitToDestroyEntity({ GetEntity(id) });
			}
		}

		uint64_t Scene_InstantiatePrefab(AssetHandle* aPrefabHandle)
		{
			return Scene_InstantiatePrefabWithTransform(aPrefabHandle, nullptr, nullptr, nullptr);
		}

		uint64_t Scene_InstantiatePrefabWithTranslation(AssetHandle* aPrefabHandle, CU::Vector3f* aTranslation)
		{
			return Scene_InstantiatePrefabWithTransform(aPrefabHandle, aTranslation, nullptr, nullptr);
		}

		uint64_t Scene_InstantiatePrefabWithTranslationAndRotation(AssetHandle* aPrefabHandle, CU::Vector3f* aTranslation, CU::Vector3f* aRotation)
		{
			return Scene_InstantiatePrefabWithTransform(aPrefabHandle, aTranslation, aRotation, nullptr);
		}

		uint64_t Scene_InstantiatePrefabWithTransform(AssetHandle* aPrefabHandle, CU::Vector3f* aTranslation, CU::Vector3f* aRotation, CU::Vector3f* aScale)
		{
			std::shared_ptr<Scene> scene = ScriptEngine::GetSceneContext();
			EPOCH_ASSERT(scene, "No active scene!");

			std::shared_ptr<Prefab> prefab = AssetManager::GetAsset<Prefab>(*aPrefabHandle);
			if (prefab == nullptr)
			{
				LOG_ERROR_TAG("C#", "Cannot instantiate prefab. No prefab with handle {} found.", *aPrefabHandle);
				return 0;
			}

			return scene->Instantiate(prefab, aTranslation, aRotation, aScale).GetUUID();
		}

		uint64_t Scene_InstantiatePrefabWithParent(AssetHandle* aPrefabHandle, uint64_t aParentID)
		{
			return Scene_InstantiatePrefabWithTransformWithParent(aPrefabHandle, aParentID, nullptr, nullptr, nullptr);
		}

		uint64_t Scene_InstantiatePrefabWithTranslationWithParent(AssetHandle* aPrefabHandle, uint64_t aParentID, CU::Vector3f* aTranslation)
		{
			return Scene_InstantiatePrefabWithTransformWithParent(aPrefabHandle, aParentID, aTranslation, nullptr, nullptr);
		}

		uint64_t Scene_InstantiatePrefabWithTranslationAndRotationWithParent(AssetHandle* aPrefabHandle, uint64_t aParentID, CU::Vector3f* aTranslation, CU::Vector3f* aRotation)
		{
			return Scene_InstantiatePrefabWithTransformWithParent(aPrefabHandle, aParentID, aTranslation, aRotation, nullptr);
		}

		uint64_t Scene_InstantiatePrefabWithTransformWithParent(AssetHandle* aPrefabHandle, uint64_t aParentID, CU::Vector3f* aTranslation, CU::Vector3f* aRotation, CU::Vector3f* aScale)
		{
			std::shared_ptr<Scene> scene = ScriptEngine::GetSceneContext();
			EPOCH_ASSERT(scene, "No active scene!");
			Entity parent = scene->TryGetEntityWithUUID(aParentID);
			EPOCH_ASSERT(parent, "Invalid parent id!");

			std::shared_ptr<Prefab> prefab = AssetManager::GetAsset<Prefab>(*aPrefabHandle);
			if (prefab == nullptr)
			{
				LOG_ERROR_TAG("C#", "Cannot instantiate prefab. No prefab with handle {} found.", *aPrefabHandle);
				return 0;
			}

			return scene->InstantiateChild(prefab, parent, aTranslation, aRotation, aScale).GetUUID();
		}

#pragma endregion
		
#pragma region Entity

		bool Entity_GetIsActive(uint64_t aEntityID)
		{
			auto entity = GetEntity(aEntityID);
			return entity ? entity.IsActive() : false;
		}

		void Entity_SetIsActive(uint64_t aEntityID, bool aState)
		{
			auto entity = GetEntity(aEntityID);
			if (entity)
			{
				entity.SetIsActive(aState);
			}
		}

		uint64_t Entity_GetParent(uint64_t aEntityID)
		{
			auto entity = GetEntity(aEntityID);
			return entity ? entity.GetParentUUID() : UUID(0);
		}

		void Entity_SetParent(uint64_t aEntityID, uint64_t aParentID)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;

			if (aParentID == 0)
			{
				ScriptEngine::GetSceneContext()->UnparentEntity(entity);
			}
			else
			{
				Entity parent = GetEntity(aParentID);
				if (!entity) return;
				entity.SetParent(parent);
			}
		}

		MonoArray* Entity_GetChildren(uint64_t aEntityID)
		{
			std::shared_ptr<Scene> scene = ScriptEngine::GetSceneContext();
			EPOCH_ASSERT(scene, "No active scene!");
			Entity parent = GetEntity(aEntityID);
			if (!parent) return 0;

			const auto& children = parent.Children();
			MonoArray* result = ManagedArrayUtils::Create<Entity>(children.size());

			for (size_t i = 0; i < children.size(); i++)
			{
				ManagedArrayUtils::SetValue(result, i, children[i]);
			}

			return result;
		}

		uint64_t Entity_GetChildByName(uint64_t aParentID, MonoString* aName)
		{
			std::shared_ptr<Scene> scene = ScriptEngine::GetSceneContext();
			EPOCH_ASSERT(scene, "No active scene!");
			Entity parent = GetEntity(aParentID);
			if (!parent) return 0;

			const std::string name = ScriptUtils::MonoStringToUTF8(aName);

			UUID result = 0;
			for (auto child : parent.GetChildren())
			{
				if (child.GetName() == name)
				{
					result = child.GetUUID();
					break;
				}
			}
			return result;
		}

		void Entity_AddComponent(uint64_t entityID, MonoReflectionType* componentType)
		{
			if (componentType == nullptr)
			{
				LOG_ERROR_TAG("C#", "Attempting to add a component of null type to an entity.");
				return;
			}

			MonoType* managedComponentType = mono_reflection_type_get_type(componentType);
			char* componentTypeName = mono_type_get_name(managedComponentType);

			auto entity = GetEntity(entityID);
			if (!entity) return;

			if (staticAddComponentFuncs.find(managedComponentType) == staticAddComponentFuncs.end())
			{
				LOG_ERROR_TAG("C#", "Cannot add component of type '{}' to entity '{}'. That component hasn't been registered by the engine.", componentTypeName, entity.GetName());
				mono_free(componentTypeName);
				return;
			}

			if (staticHasComponentFuncs.at(managedComponentType)(entity))
			{
				LOG_ERROR_TAG("C#", "Attempting to add duplicate component '{}' to entity '{}', ignoring.", componentTypeName, entity.GetName());
				mono_free(componentTypeName);
				return;
			}

			staticAddComponentFuncs.at(managedComponentType)(entity);
			mono_free(componentTypeName);
		}

		bool Entity_HasComponent(uint64_t entityID, MonoReflectionType* componentType)
		{
			if (componentType == nullptr)
			{
				LOG_ERROR_TAG("C#", "Attempting to check if entity has a component of null type.");
				return false;
			}

			auto entity = GetEntity(entityID);
			if (!entity) return false;

			MonoType* managedType = mono_reflection_type_get_type(componentType);

			if (staticHasComponentFuncs.find(managedType) == staticHasComponentFuncs.end())
			{
				char* componentTypeName = mono_type_get_name(managedType);
				LOG_ERROR_TAG("C#", "Cannot check if entity '{}' has a component of type '{}'. That component hasn't been registered by the engine.", entity.GetName(), componentTypeName);
				mono_free(componentTypeName);
				return false;
			}

			return staticHasComponentFuncs.at(managedType)(entity);
		}

		bool Entity_RemoveComponent(uint64_t entityID, MonoReflectionType* componentType)
		{
			if (componentType == nullptr)
			{
				LOG_ERROR_TAG("C#", "Attempting to remove a component of null type from an entity.");
				return false;
			}

			auto entity = GetEntity(entityID);
			if (!entity) return false;

			MonoType* managedType = mono_reflection_type_get_type(componentType);
			char* componentTypeName = mono_type_get_name(managedType);
			if (staticRemoveComponentFuncs.find(managedType) == staticRemoveComponentFuncs.end())
			{
				LOG_ERROR_TAG("C#", "Cannot remove a component of type '{}' from entity '{}'. That component hasn't been registered by the engine.", componentTypeName, entity.GetName());
				return false;
			}

			if (!staticHasComponentFuncs.at(managedType)(entity))
			{
				LOG_ERROR_TAG("C#", "Tried to remove component '{}' from entity '{}' even though it doesn't have that component.", componentTypeName, entity.GetName());
				return false;
			}

			mono_free(componentTypeName);
			staticRemoveComponentFuncs.at(managedType)(entity);
			return true;
		}
		
#pragma region TagComponent

		MonoString* NameComponent_GetName(uint64_t aEntityID)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return nullptr;

			const auto& nameComponent = entity.GetComponent<NameComponent>();
			return ScriptUtils::UTF8StringToMono(nameComponent.name);
		}

		void NameComponent_SetName(uint64_t aEntityID, MonoString* aName)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;

			auto& nameComponent = entity.GetComponent<NameComponent>();
			nameComponent.name = ScriptUtils::MonoStringToUTF8(aName);
		}
		
#pragma endregion

#pragma endregion
		
#pragma region TransformComponent

		void TransformComponent_GetTransform(uint64_t aEntityID, Transform* outTransform)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;

			const CU::Transform& trans = entity.GetComponent<TransformComponent>().transform;
			outTransform->translation = trans.GetTranslation();
			outTransform->rotation = trans.GetRotation();
			outTransform->scale = trans.GetScale();
		}

		void TransformComponent_SetTransform(uint64_t aEntityID, Transform* aTransform)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!aTransform) return;

			CU::Transform& trans = entity.GetComponent<TransformComponent>().transform;
			trans.SetTranslation(aTransform->translation);
			trans.SetRotation(aTransform->rotation);
			trans.SetScale(aTransform->scale);
		}

		void TransformComponent_GetWorldSpaceTransform(uint64_t aEntityID, Transform* outTransform)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			
			const CU::Transform worldTrans = ScriptEngine::GetSceneContext()->GetWorldSpaceTransform(entity);
			outTransform->translation = worldTrans.GetTranslation();
			outTransform->rotation = worldTrans.GetRotation();
			outTransform->scale = worldTrans.GetScale();
		}

		void TransformComponent_GetTranslation(uint64_t aEntityID, CU::Vector3f* outTranslation)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;

			*outTranslation = entity.GetComponent<TransformComponent>().transform.GetTranslation();
		}

		void TransformComponent_GetRotation(uint64_t aEntityID, CU::Vector3f* outRotation)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;

			*outRotation = entity.GetComponent<TransformComponent>().transform.GetRotation();
		}

		void TransformComponent_GetScale(uint64_t aEntityID, CU::Vector3f* outScale)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;

			*outScale = entity.GetComponent<TransformComponent>().transform.GetScale();
		}

		void TransformComponent_SetTranslation(uint64_t aEntityID, CU::Vector3f* aTranslation)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!aTranslation) return;
			
			entity.GetComponent<TransformComponent>().transform.SetTranslation(*aTranslation);
		}

		void TransformComponent_SetRotation(uint64_t aEntityID, CU::Vector3f* aRotation)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!aRotation) return;

			entity.GetComponent<TransformComponent>().transform.SetRotation(*aRotation);
		}

		void TransformComponent_SetScale(uint64_t aEntityID, CU::Vector3f* aScale)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!aScale) return;
			
			entity.GetComponent<TransformComponent>().transform.SetScale(*aScale);
		}

		void TransformComponent_Translate(uint64_t aEntityID, CU::Vector3f* aTranslation)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (aTranslation == nullptr) return;

			entity.Transform().Translate(*aTranslation);
		}

		void TransformComponent_Rotate(uint64_t aEntityID, CU::Vector3f* aRotation)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (aRotation == nullptr) return;

			entity.Transform().Rotate(*aRotation);
		}

		void TransformComponent_RotateAround(uint64_t aEntityID, CU::Vector3f* aPoint, CU::Vector3f* aAxis, float aAngle)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (aPoint == nullptr) return;
			if (aAxis == nullptr) return;

			entity.Transform().RotateAround(*aPoint, *aAxis, aAngle);
		}

		void TransformComponent_LookAt(uint64_t aEntityID, CU::Vector3f* aTarget, CU::Vector3f* aUp)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (aTarget == nullptr) return;
			if (aUp == nullptr) return;
			
			entity.Transform().LookAt(*aTarget, *aUp);
		}

#pragma endregion

#pragma region ScriptComponent
		
		MonoObject* ScriptComponent_GetInstance(uint64_t aEntityID)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return nullptr;
			if (!entity.HasComponent<ScriptComponent>()) return nullptr;

			const auto& component = entity.GetComponent<ScriptComponent>();

			if (!ScriptEngine::IsModuleValid(component.scriptClassHandle))
			{
				LOG_ERROR("Entity is referencing an invalid C# class!");
				return nullptr;
			}

			if (!ScriptEngine::IsEntityInstantiated(entity))
			{
				if (ScriptEngine::IsEntityInstantiated(entity, false))
				{
					ScriptEngine::CallMethod(component.managedInstance, "OnCreate");

					
					entity.GetComponent<ScriptComponent>().isRuntimeInitialized = true;

					return GCManager::GetReferencedObject(component.managedInstance);
				}
				else if (component.managedInstance == nullptr)
				{
					ScriptEngine::RuntimeInitializeScriptEntity(entity);
					return GCManager::GetReferencedObject(component.managedInstance);
				}

				LOG_ERROR("Entity '{}' isn't instantiated?", entity.GetName());
				return nullptr;
			}

			return GCManager::GetReferencedObject(component.managedInstance);
		}

#pragma endregion

#pragma region TextRendererComponent

		MonoString* TextComponent_GetText(uint64_t aEntityID)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return nullptr;
			if (!entity.HasComponent<TextRendererComponent>()) return nullptr;

			const auto& component = entity.GetComponent<TextRendererComponent>();
			return ScriptUtils::UTF8StringToMono(component.text);
		}

		void TextComponent_SetText(uint64_t aEntityID, MonoString* aText)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<TextRendererComponent>()) return;

			auto& component = entity.GetComponent<TextRendererComponent>();
			component.text = ScriptUtils::MonoStringToUTF8(aText);
		}

		void TextComponent_GetColor(uint64_t aEntityID, CU::Color* outColor)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<TextRendererComponent>()) return;

			const auto& component = entity.GetComponent<TextRendererComponent>();
			*outColor = component.color;
		}

		void TextComponent_SetColor(uint64_t aEntityID, CU::Color* aColor)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<TextRendererComponent>()) return;

			auto& component = entity.GetComponent<TextRendererComponent>();
			component.color = *aColor;
		}

#pragma endregion

#pragma region PointLightComponent

		void PointLightComponent_GetColor(uint64_t aEntityID, CU::Color* outColor)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<PointLightComponent>()) return;

			auto& component = entity.GetComponent<PointLightComponent>();
			*outColor = component.color;
		}

		void PointLightComponent_SetColor(uint64_t aEntityID, CU::Color* aColor)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<PointLightComponent>()) return;

			auto& component = entity.GetComponent<PointLightComponent>();
			component.color = *aColor;
		}

		float PointLightComponent_GetIntensity(uint64_t aEntityID)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return 0.0f;
			if (!entity.HasComponent<PointLightComponent>()) return 0.0f;

			auto& component = entity.GetComponent<PointLightComponent>();
			return component.intensity;
		}

		void PointLightComponent_SetIntensity(uint64_t aEntityID, float aIntensity)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<PointLightComponent>()) return;

			auto& component = entity.GetComponent<PointLightComponent>();
			component.intensity = aIntensity;
		}

		bool PointLightComponent_GetCastsShadows(uint64_t aEntityID)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return false;
			if (!entity.HasComponent<PointLightComponent>()) return false;

			auto& component = entity.GetComponent<PointLightComponent>();
			return component.castsShadows;
		}

		void PointLightComponent_SetCastsShadows(uint64_t aEntityID, bool aState)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<PointLightComponent>()) return;

			auto& component = entity.GetComponent<PointLightComponent>();
			component.castsShadows = aState;
		}

		float PointLightComponent_GetRange(uint64_t aEntityID)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return 0.0f;
			if (!entity.HasComponent<PointLightComponent>()) return 0.0f;

			auto& component = entity.GetComponent<PointLightComponent>();
			return component.range;
		}

		void PointLightComponent_SetRange(uint64_t aEntityID, float aRange)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<PointLightComponent>()) return;

			auto& component = entity.GetComponent<PointLightComponent>();
			component.range = aRange;
		}

#pragma endregion

#pragma region SpotlightComponent

		void SpotlightComponent_GetColor(uint64_t aEntityID, CU::Color* outColor)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<SpotlightComponent>()) return;

			auto& component = entity.GetComponent<SpotlightComponent>();
			*outColor = component.color;
		}

		void SpotlightComponent_SetColor(uint64_t aEntityID, CU::Color* aColor)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<SpotlightComponent>()) return;

			auto& component = entity.GetComponent<SpotlightComponent>();
			component.color = *aColor;
		}

		float SpotlightComponent_GetIntensity(uint64_t aEntityID)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return 0.0f;
			if (!entity.HasComponent<SpotlightComponent>()) return 0.0f;

			auto& component = entity.GetComponent<SpotlightComponent>();
			return component.intensity;
		}

		void SpotlightComponent_SetIntensity(uint64_t aEntityID, float aIntensity)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<SpotlightComponent>()) return;

			auto& component = entity.GetComponent<SpotlightComponent>();
			component.intensity = aIntensity;
		}

		bool SpotlightComponent_GetCastsShadows(uint64_t aEntityID)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return false;
			if (!entity.HasComponent<SpotlightComponent>()) return false;

			auto& component = entity.GetComponent<SpotlightComponent>();
			return component.castsShadows;
		}

		void SpotlightComponent_SetCastsShadows(uint64_t aEntityID, bool aState)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<SpotlightComponent>()) return;

			auto& component = entity.GetComponent<SpotlightComponent>();
			component.castsShadows = aState;
		}

		float SpotlightComponent_GetRange(uint64_t aEntityID)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return 0.0f;
			if (!entity.HasComponent<SpotlightComponent>()) return 0.0f;

			auto& component = entity.GetComponent<SpotlightComponent>();
			return component.range;
		}

		void SpotlightComponent_SetRange(uint64_t aEntityID, float aRange)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<SpotlightComponent>()) return;

			auto& component = entity.GetComponent<SpotlightComponent>();
			component.range = aRange;
		}

		float SpotlightComponent_GetOuterAngle(uint64_t aEntityID)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return 0.0f;
			if (!entity.HasComponent<SpotlightComponent>()) return 0.0f;

			auto& component = entity.GetComponent<SpotlightComponent>();
			return component.outerSpotAngle * CU::Math::ToDeg;
		}

		void SpotlightComponent_SetOuterAngle(uint64_t aEntityID, float aAngle)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<SpotlightComponent>()) return;

			auto& component = entity.GetComponent<SpotlightComponent>();
			component.outerSpotAngle = aAngle * CU::Math::ToRad;
		}

		float SpotlightComponent_GetInnerAngle(uint64_t aEntityID)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return 0.0f;
			if (!entity.HasComponent<SpotlightComponent>()) return 0.0f;

			auto& component = entity.GetComponent<SpotlightComponent>();
			return component.innerSpotAngle * CU::Math::ToDeg;
		}

		void SpotlightComponent_SetInnerAngle(uint64_t aEntityID, float aAngle)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<SpotlightComponent>()) return;

			auto& component = entity.GetComponent<SpotlightComponent>();
			component.innerSpotAngle = aAngle * CU::Math::ToRad;
		}

#pragma endregion

#pragma region Log

		void Log_LogMessage(LogLevel aLevel, MonoString* aInFormattedMessage)
		{
			std::string message = ScriptUtils::MonoStringToUTF8(aInFormattedMessage);
			switch (aLevel)
			{
				case LogLevel::Debug:
					CONSOLE_LOG_DEBUG(message);
					break;
				case LogLevel::Info:
					CONSOLE_LOG_INFO(message);
					break;
				case LogLevel::Warn:
					CONSOLE_LOG_WARN(message);
					break;
				case LogLevel::Error:
					CONSOLE_LOG_ERROR(message);
					break;
			}
		}

#pragma endregion

#pragma region Gizmos

		void Gizmos_DrawWireSphere(CU::Vector3f* aCenter, float aRadius, CU::Color* aColor)
		{
			std::shared_ptr<DebugRenderer> dr = ScriptEngine::GetSceneRenderer()->GetDebugRenderer();
			if (dr)
			{
				dr->DrawWireSphere(*aCenter, CU::Vector3f::Zero, aRadius, *aColor);
			}
		}

		void Gizmos_DrawWireCube(CU::Vector3f* aCenter, CU::Vector3f* aRotation, CU::Vector3f* aSize, CU::Color* aColor)
		{
			std::shared_ptr<DebugRenderer> dr = ScriptEngine::GetSceneRenderer()->GetDebugRenderer();
			if (dr)
			{
				dr->DrawWireBox(*aCenter, *aRotation, *aSize, *aColor);
			}
		}

		void Gizmos_DrawLine(CU::Vector3f* aFrom, CU::Vector3f* aTo, CU::Color* aColor)
		{
			std::shared_ptr<DebugRenderer> dr = ScriptEngine::GetSceneRenderer()->GetDebugRenderer();
			if (dr)
			{
				dr->DrawLine(*aFrom, *aTo, *aColor);
			}
		}

#pragma endregion

#pragma region Input

		bool Input_IsKeyPressed(KeyCode aKeyCode) { return Input::IsKeyPressed(aKeyCode); }
		bool Input_IsKeyHeld(KeyCode aKeyCode) { return Input::IsKeyHeld(aKeyCode); }
		bool Input_IsKeyReleased(KeyCode aKeyCode) { return Input::IsKeyReleased(aKeyCode); }

		bool Input_IsMouseButtonPressed(MouseButton aButton)
		{
			bool isPressed = Input::IsMouseButtonPressed(aButton);

			const bool enableImGui = Application::Get().GetSpecification().enableImGui;
			if (isPressed && enableImGui && GImGui->HoveredWindow != nullptr)
			{
				// Make sure we're in the viewport panel
				ImGuiWindow* viewportWindow = ImGui::FindWindowByName("Viewport");
				if (viewportWindow != nullptr)
				{
					isPressed = GImGui->HoveredWindow->ID == viewportWindow->ID;
				}
			}

			return isPressed;
		}

		bool Input_IsMouseButtonHeld(MouseButton aButton)
		{
			bool isHeld = Input::IsMouseButtonHeld(aButton);

			const bool enableImGui = Application::Get().GetSpecification().enableImGui;
			if (isHeld && enableImGui && GImGui->HoveredWindow != nullptr)
			{
				// Make sure we're in the viewport panel
				ImGuiWindow* viewportWindow = ImGui::FindWindowByName("Viewport");
				if (viewportWindow != nullptr)
				{
					isHeld = GImGui->HoveredWindow->ID == viewportWindow->ID;
				}
			}

			return isHeld;
		}

		bool Input_IsMouseButtonReleased(MouseButton aButton)
		{
			bool released = Input::IsMouseButtonReleased(aButton);

			const bool enableImGui = Application::Get().GetSpecification().enableImGui;
			if (released && enableImGui && GImGui->HoveredWindow != nullptr)
			{
				// Make sure we're in the viewport panel
				ImGuiWindow* viewportWindow = ImGui::FindWindowByName("Viewport");
				if (viewportWindow != nullptr)
				{
					released = GImGui->HoveredWindow->ID == viewportWindow->ID;
				}
			}

			return released;
		}

		void Input_GetMouseDelta(CU::Vector2f* outDelta) { *outDelta = Input::GetMouseDelta(); }
		
		CursorMode Input_GetCursorMode() { return Input::GetCursorMode(); }
		void Input_SetCursorMode(CursorMode aMode) { Input::SetCursorMode(aMode); }

		void Input_GetScrollDelta(CU::Vector2f* outDelta) { *outDelta = Input::GetMouseScroll(); }

		bool Input_IsGamepadButtonPressed(GamepadButton aButton) { return Input::IsControllerButtonPressed(0, aButton); }
		bool Input_IsGamepadButtonHeld(GamepadButton aButton) { return Input::IsControllerButtonHeld(0, aButton); }
		bool Input_IsGamepadButtonReleased(GamepadButton aButton) { return Input::IsControllerButtonReleased(0, aButton); }

		float Input_GetGamepadAxis(GamepadAxis aAxis) { return Input::GetControllerAxis(0, aAxis); }
		
#pragma endregion

#pragma region Physics

		bool Physics_Raycast(CU::Vector3f* aOrigin, CU::Vector3f* aDirection, float aMaxDistance, HitInfo* outHitInfo)
		{
			std::shared_ptr<Scene> scene = ScriptEngine::GetSceneContext();
			
			bool hit = scene->GetPhysicsScene()->Raycast(*aOrigin, *aDirection, aMaxDistance, outHitInfo);

			return hit;
		}

		void Physics_GetGravity(CU::Vector3f* outGravity)
		{
			std::shared_ptr<Scene> scene = ScriptEngine::GetSceneContext();
			*outGravity = scene->GetPhysicsScene()->GetGravity();
		}

		void Physics_SetGravity(CU::Vector3f* aGravity)
		{
			std::shared_ptr<Scene> scene = ScriptEngine::GetSceneContext();
			scene->GetPhysicsScene()->SetGravity(*aGravity);
		}

		void Physics_AddRadialImpulse(CU::Vector3f* aOrigin, float aRadius, float aStrength)
		{
			std::shared_ptr<Scene> scene = ScriptEngine::GetSceneContext();
			scene->GetPhysicsScene()->AddRadialImpulse(*aOrigin, aRadius, aStrength);
		}

#pragma endregion

#pragma region RigidbodyComponent

		float RigidbodyComponent_GetMass(uint64_t aEntityID)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return 0.0f;
			if (!entity.HasComponent<RigidbodyComponent>()) return 0.0f;

			std::shared_ptr<PhysicsBody> physicsBody = GetPhysicsBody(aEntityID);
			if (!physicsBody)
			{
				LOG_ERROR("Couldn't find physics body for entity '{}'", entity.GetName());
				return 0.0f;
			}
			
			return physicsBody->GetMass();
		}

		void RigidbodyComponent_SetMass(uint64_t aEntityID, float aMass)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<RigidbodyComponent>()) return;
			
			std::shared_ptr<PhysicsBody> physicsBody = GetPhysicsBody(aEntityID);
			if (!physicsBody)
			{
				LOG_ERROR("Couldn't find physics body for entity '{}'", entity.GetName());
				return;
			}
			
			physicsBody->SetMass(aMass);

			//RigidbodyComponent& rigidbodyComp = entity.GetComponent<RigidbodyComponent>();
			//rigidbodyComp.mass = aMass;
		}


		bool RigidbodyComponent_GetUseGravity(uint64_t aEntityID)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return true;
			if (!entity.HasComponent<RigidbodyComponent>()) return true;
			
			std::shared_ptr<PhysicsBody> physicsBody = GetPhysicsBody(aEntityID);
			if (!physicsBody)
			{
				LOG_ERROR("Couldn't find physics body for entity '{}'", entity.GetName());
				return true;
			}
			
			return physicsBody->GetUseGravity();
		}

		void RigidbodyComponent_SetUseGravity(uint64_t aEntityID, bool aState)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<RigidbodyComponent>()) return;
			
			std::shared_ptr<PhysicsBody> physicsBody = GetPhysicsBody(aEntityID);
			if (!physicsBody)
			{
				LOG_ERROR("Couldn't find physics body for entity '{}'", entity.GetName());
				return;
			}
			
			physicsBody->SetUseGravity(aState);
		}


		float RigidbodyComponent_GetDrag(uint64_t aEntityID)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return 0.0f;
			if (!entity.HasComponent<RigidbodyComponent>()) return 0.0f;
			
			std::shared_ptr<PhysicsBody> physicsBody = GetPhysicsBody(aEntityID);
			if (!physicsBody)
			{
				LOG_ERROR("Couldn't find physics body for entity '{}'", entity.GetName());
				return physicsBody->GetDrag();
			}

			return 0.0f;
		}

		void RigidbodyComponent_SetDrag(uint64_t aEntityID, float aDrag)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<RigidbodyComponent>()) return;
			
			std::shared_ptr<PhysicsBody> physicsBody = GetPhysicsBody(aEntityID);
			if (!physicsBody)
			{
				LOG_ERROR("Couldn't find physics body for entity '{}'", entity.GetName());
				return;
			}

			physicsBody->SetDrag(aDrag);
		}


		float RigidbodyComponent_GetAngularDrag(uint64_t aEntityID)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return 0.0f;
			if (!entity.HasComponent<RigidbodyComponent>()) return 0.0f;
			
			std::shared_ptr<PhysicsBody> physicsBody = GetPhysicsBody(aEntityID);
			if (!physicsBody)
			{
				LOG_ERROR("Couldn't find physics body for entity '{}'", entity.GetName());
				return 0.0f;
			}

			return physicsBody->GetAngularDrag();
		}

		void RigidbodyComponent_SetAngularDrag(uint64_t aEntityID, float aDrag)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<RigidbodyComponent>()) return;
			
			std::shared_ptr<PhysicsBody> physicsBody = GetPhysicsBody(aEntityID);
			if (!physicsBody)
			{
				LOG_ERROR("Couldn't find physics body for entity '{}'", entity.GetName());
				return;
			}
			
			physicsBody->SetAngularDrag(aDrag);
		}


		void RigidbodyComponent_GetVelocity(uint64_t aEntityID, CU::Vector3f* outVelocity)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<RigidbodyComponent>()) return;
			
			std::shared_ptr<PhysicsBody> physicsBody = GetPhysicsBody(aEntityID);
			if (!physicsBody)
			{
				LOG_ERROR("Couldn't find physics body for entity '{}'", entity.GetName());
				return;
			}

			*outVelocity = physicsBody->GetVelocity();
		}

		void RigidbodyComponent_SetVelocity(uint64_t aEntityID, CU::Vector3f* aVelocity)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<RigidbodyComponent>()) return;
			
			std::shared_ptr<PhysicsBody> physicsBody = GetPhysicsBody(aEntityID);
			if (!physicsBody)
			{
				LOG_ERROR("Couldn't find physics body for entity '{}'", entity.GetName());
				return;
			}

			physicsBody->SetVelocity(*aVelocity);
		}


		void RigidbodyComponent_GetAngularVelocity(uint64_t aEntityID, CU::Vector3f* outVelocity)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<RigidbodyComponent>()) return;
			
			std::shared_ptr<PhysicsBody> physicsBody = GetPhysicsBody(aEntityID);
			if (!physicsBody)
			{
				LOG_ERROR("Couldn't find physics body for entity '{}'", entity.GetName());
				return;
			}

			*outVelocity = physicsBody->GetAngularVelocity();
		}

		void RigidbodyComponent_SetAngularVelocity(uint64_t aEntityID, CU::Vector3f* aVelocity)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<RigidbodyComponent>()) return;
			
			std::shared_ptr<PhysicsBody> physicsBody = GetPhysicsBody(aEntityID);
			if (!physicsBody)
			{
				LOG_ERROR("Couldn't find physics body for entity '{}'", entity.GetName());
				return;
			}
			
			physicsBody->SetAngularVelocity(*aVelocity);
		}


		void RigidbodyComponent_GetPosition(uint64_t aEntityID, CU::Vector3f* outPosition)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<RigidbodyComponent>()) return;
			
			std::shared_ptr<PhysicsBody> physicsBody = GetPhysicsBody(aEntityID);
			if (!physicsBody)
			{
				LOG_ERROR("Couldn't find physics body for entity '{}'", entity.GetName());
				return;
			}

			*outPosition = physicsBody->GetPosition();
		}

		void RigidbodyComponent_SetPosition(uint64_t aEntityID, CU::Vector3f* aPosition)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<RigidbodyComponent>()) return;
			
			std::shared_ptr<PhysicsBody> physicsBody = GetPhysicsBody(aEntityID);
			if (!physicsBody)
			{
				LOG_ERROR("Couldn't find physics body for entity '{}'", entity.GetName());
				return;
			}

			physicsBody->SetPosition(*aPosition);
		}


		void RigidbodyComponent_GetRotation(uint64_t aEntityID, CU::Vector3f* outRotation)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<RigidbodyComponent>()) return;
			
			std::shared_ptr<PhysicsBody> physicsBody = GetPhysicsBody(aEntityID);
			if (!physicsBody)
			{
				LOG_ERROR("Couldn't find physics body for entity '{}'", entity.GetName());
				return;
			}

			*outRotation = physicsBody->GetRotation();
		}

		void RigidbodyComponent_SetRotation(uint64_t aEntityID, CU::Vector3f* aRotation)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<RigidbodyComponent>()) return;
			
			std::shared_ptr<PhysicsBody> physicsBody = GetPhysicsBody(aEntityID);
			if (!physicsBody)
			{
				LOG_ERROR("Couldn't find physics body for entity '{}'", entity.GetName());
				return;
			}

			physicsBody->SetRotation(*aRotation);
		}


		PhysicsAxis RigidbodyComponent_GetConstraints(uint64_t aEntityID)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return PhysicsAxis::None;
			if (!entity.HasComponent<RigidbodyComponent>()) return PhysicsAxis::None;
			
			std::shared_ptr<PhysicsBody> physicsBody = GetPhysicsBody(aEntityID);
			if (!physicsBody)
			{
				LOG_ERROR("Couldn't find physics body for entity '{}'", entity.GetName());
				return PhysicsAxis::None;
			}

			return physicsBody->GetConstraints();
		}

		void RigidbodyComponent_SetConstraints(uint64_t aEntityID, PhysicsAxis aConstraints)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<RigidbodyComponent>()) return;
			
			std::shared_ptr<PhysicsBody> physicsBody = GetPhysicsBody(aEntityID);
			if (!physicsBody)
			{
				LOG_ERROR("Couldn't find physics body for entity '{}'", entity.GetName());
				return;
			}

			physicsBody->SetConstraints(aConstraints);
		}


		void RigidbodyComponent_AddForce(uint64_t aEntityID, CU::Vector3f* aForce, ForceMode aForceMode)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<RigidbodyComponent>()) return;
			
			std::shared_ptr<PhysicsBody> physicsBody = GetPhysicsBody(aEntityID);
			if (!physicsBody)
			{
				LOG_ERROR("Couldn't find physics body for entity '{}'", entity.GetName());
				return;
			}

			physicsBody->AddForce(*aForce, aForceMode);
		}

		void RigidbodyComponent_AddForceAtPosition(uint64_t aEntityID, CU::Vector3f* aForce, CU::Vector3f* aPosition, ForceMode aForceMode)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<RigidbodyComponent>()) return;
			
			std::shared_ptr<PhysicsBody> physicsBody = GetPhysicsBody(aEntityID);
			if (!physicsBody)
			{
				LOG_ERROR("Couldn't find physics body for entity '{}'", entity.GetName());
				return;
			}

			physicsBody->AddForceAtPosition(*aForce, *aPosition, aForceMode);
		}

		void RigidbodyComponent_AddTorque(uint64_t aEntityID, CU::Vector3f* aTorque, ForceMode aForceMode)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<RigidbodyComponent>()) return;
			
			std::shared_ptr<PhysicsBody> physicsBody = GetPhysicsBody(aEntityID);
			if (!physicsBody)
			{
				LOG_ERROR("Couldn't find physics body for entity '{}'", entity.GetName());
				return;
			}

			physicsBody->AddTorque(*aTorque, aForceMode);
		}


		void RigidbodyComponent_Teleport(uint64_t aEntityID, CU::Vector3f* aTargetPosition, CU::Vector3f* aTargetRotation)
		{
			std::shared_ptr<Scene> scene = ScriptEngine::GetSceneContext();
			Entity entity = GetEntity(aEntityID);
			
			if (!entity)
			{
				return;
			}

			scene->GetPhysicsScene()->Teleport(entity, *aTargetPosition, CU::Quatf(*aTargetRotation));
		}

#pragma endregion

#pragma region CharacterControllerComponent

		float CharacterControllerComponent_GetStepOffset(uint64_t aEntityID)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return 0.0f;
			if (!entity.HasComponent<CharacterControllerComponent>()) return 0.0f;
			
			std::shared_ptr<CharacterController> characterController = GetCharacterController(aEntityID);
			if (!characterController)
			{
				LOG_ERROR("Couldn't find character controller for entity '{}'", entity.GetName());
				return 0.0f;
			}

			return characterController->GetStepOffset();
		}

		void CharacterControllerComponent_SetStepOffset(uint64_t aEntityID, float aStepOffset)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<CharacterControllerComponent>()) return;
			
			std::shared_ptr<CharacterController> characterController = GetCharacterController(aEntityID);
			if (!characterController)
			{
				LOG_ERROR("Couldn't find character controller for entity '{}'", entity.GetName());
				return;
			}

			characterController->SetStepOffset(aStepOffset);
		}

		float CharacterControllerComponent_GetSlopeLimit(uint64_t aEntityID)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return 0.0f;
			if (!entity.HasComponent<CharacterControllerComponent>()) return 0.0f;
			
			std::shared_ptr<CharacterController> characterController = GetCharacterController(aEntityID);
			if (!characterController)
			{
				LOG_ERROR("Couldn't find character controller for entity '{}'", entity.GetName());
				return 0.0f;
			}

			return characterController->GetSlopeLimit();
		}

		void CharacterControllerComponent_SetSlopeLimit(uint64_t aEntityID, float aSlopeLimit)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<CharacterControllerComponent>()) return;
			
			std::shared_ptr<CharacterController> characterController = GetCharacterController(aEntityID);
			if (!characterController)
			{
				LOG_ERROR("Couldn't find character controller for entity '{}'", entity.GetName());
				return;
			}

			characterController->SetSlopeLimit(aSlopeLimit);
		}

		void CharacterControllerComponent_Resize(uint64_t aEntityID, float aHeight)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<CharacterControllerComponent>()) return;
			
			std::shared_ptr<CharacterController> characterController = GetCharacterController(aEntityID);
			if (!characterController)
			{
				LOG_ERROR("Couldn't find character controller for entity '{}'", entity.GetName());
				return;
			}

			characterController->Resize(aHeight);
		}

		void CharacterControllerComponent_Move(uint64_t aEntityID, CU::Vector3f* aDisplacement)
		{
			auto entity = GetEntity(aEntityID);
			if (!entity) return;
			if (!entity.HasComponent<CharacterControllerComponent>()) return;
			
			std::shared_ptr<CharacterController> characterController = GetCharacterController(aEntityID);
			if (!characterController)
			{
				LOG_ERROR("Couldn't find character controller for entity '{}'", entity.GetName());
				return;
			}

			characterController->Move(*aDisplacement);
		}

#pragma endregion
	}
}
