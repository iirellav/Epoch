#pragma once
#include <CommonUtilities/Math/Vector/Vector.h>
#include <CommonUtilities/Color.h>
#include "Epoch/Assets/Asset.h"
#include "Epoch/Core/KeyCodes.h"
#include "Epoch/Physics/SceneQueries.h"
#include "Epoch/Physics/PhysicsTypes.h"

extern "C"
{
	typedef struct _MonoString MonoString;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoArray MonoArray;
	typedef struct _MonoReflectionType MonoReflectionType;
}

namespace Epoch
{
	class ScriptGlue
	{
	public:
		static void RegisterGlue();

	private:
		static void RegisterComponentTypes();
		static void RegisterInternalCalls();
	};

	namespace InternalCalls
	{
		struct Transform
		{
			CU::Vector3f translation;
			CU::Vector3f rotation;
			CU::Vector3f scale;
		};
		
#pragma region Application

		void Application_Quit();

		bool Application_GetIsVSync();
		void Application_SetIsVSync(bool aState);

		uint32_t Application_GetWidth();
		uint32_t Application_GetHeight();

#pragma endregion
		
#pragma region Noise

		void Noise_SetSeed(int aSeed);

		float Noise_SimplexNoise(float x, float y);
		float Noise_PerlinNoise(float x, float y);

#pragma endregion

#pragma region Time

		float Time_GetTimeScale();
		void Time_SetTimeScale(float aTimeScale);

#pragma endregion

#pragma region AssetHandle
		
		bool AssetHandle_IsValid(AssetHandle* aAssetHandle);

#pragma endregion

#pragma region SceneManager
		
		void SceneManager_LoadScene(AssetHandle* aSceneHandle);
		MonoString* SceneManager_GetCurrentSceneName();

#pragma endregion

#pragma region Scene

		bool Scene_IsEntityValid(uint64_t aEntityID);

		uint64_t Scene_GetEntityByName(MonoString* aName);

		uint64_t Scene_CreateEntity(MonoString* aName);
		void Scene_DestroyEntity(uint64_t aEntityID);
		void Scene_DestroyAllChildren(uint64_t aEntityID);

		uint64_t Scene_InstantiatePrefab(AssetHandle* aPrefabHandle);
		uint64_t Scene_InstantiatePrefabWithTranslation(AssetHandle* aPrefabHandle, CU::Vector3f* aTranslation);
		uint64_t Scene_InstantiatePrefabWithTranslationAndRotation(AssetHandle* aPrefabHandle, CU::Vector3f* aTranslation, CU::Vector3f* aRotation);
		uint64_t Scene_InstantiatePrefabWithTransform(AssetHandle* aPrefabHandle, CU::Vector3f* aTranslation, CU::Vector3f* aRotation, CU::Vector3f* aScale);

		uint64_t Scene_InstantiatePrefabWithParent(AssetHandle* aPrefabHandle, uint64_t aParentID);
		uint64_t Scene_InstantiatePrefabWithTranslationWithParent(AssetHandle* aPrefabHandle, uint64_t aParentID, CU::Vector3f* aTranslation);
		uint64_t Scene_InstantiatePrefabWithTranslationAndRotationWithParent(AssetHandle* aPrefabHandle, uint64_t aParentID, CU::Vector3f* aTranslation, CU::Vector3f* aRotation);
		uint64_t Scene_InstantiatePrefabWithTransformWithParent(AssetHandle* aPrefabHandle, uint64_t aParentID, CU::Vector3f* aTranslation, CU::Vector3f* aRotation, CU::Vector3f* aScale);

#pragma endregion
		
#pragma region Entity
		
		bool Entity_GetIsActive(uint64_t aEntityID);
		void Entity_SetIsActive(uint64_t aEntityID, bool aState);

		uint64_t Entity_GetParent(uint64_t aEntityID);
		void Entity_SetParent(uint64_t aEntityID, uint64_t aParentID);
		MonoArray* Entity_GetChildren(uint64_t aEntityID);
		uint64_t Entity_GetChildByName(uint64_t aParentID, MonoString* aName);

		void Entity_AddComponent(uint64_t aEntityID, MonoReflectionType* aComponentType);
		bool Entity_HasComponent(uint64_t aEntityID, MonoReflectionType* aComponentType);
		bool Entity_RemoveComponent(uint64_t aEntityID, MonoReflectionType* aComponentType);

#pragma endregion

#pragma region NameComponent

		MonoString* NameComponent_GetName(uint64_t aEntityID);
		void NameComponent_SetName(uint64_t aEntityID, MonoString* aName);

#pragma endregion

#pragma region TransformComponent

		void TransformComponent_GetTransform(uint64_t aEntityID, Transform* outTransform);
		void TransformComponent_SetTransform(uint64_t aEntityID, Transform* aTransform);
		void TransformComponent_GetWorldSpaceTransform(uint64_t aEntityID, Transform* outTransform);

		void TransformComponent_GetTranslation(uint64_t aEntityID, CU::Vector3f* outTranslation);
		void TransformComponent_GetRotation(uint64_t aEntityID, CU::Vector3f* outRotation);
		void TransformComponent_GetScale(uint64_t aEntityID, CU::Vector3f* outScale);
		
		void TransformComponent_SetTranslation(uint64_t aEntityID, CU::Vector3f* aTranslation);
		void TransformComponent_SetRotation(uint64_t aEntityID, CU::Vector3f* aRotation);
		void TransformComponent_SetScale(uint64_t aEntityID, CU::Vector3f* aScale);

		void TransformComponent_Translate(uint64_t aEntityID, CU::Vector3f* aTranslation);
		void TransformComponent_Rotate(uint64_t aEntityID, CU::Vector3f* aRotation);

		void TransformComponent_RotateAround(uint64_t aEntityID, CU::Vector3f* aPoint, CU::Vector3f* aAxis, float aAngle);
		void TransformComponent_LookAt(uint64_t aEntityID, CU::Vector3f* aTarget, CU::Vector3f* aUp);

#pragma endregion

#pragma region MeshRendererComponent

		bool MeshRendererComponent_GetIsActive(uint64_t aEntityID);
		void MeshRendererComponent_SetIsActive(uint64_t aEntityID, bool aState);

		bool MeshRendererComponent_GetMesh(uint64_t aEntityID, AssetHandle* outHandle);
		void MeshRendererComponent_SetMesh(uint64_t aEntityID, AssetHandle* aHandle);

		bool MeshRendererComponent_GetCastsShadows(uint64_t aEntityID);
		void MeshRendererComponent_SetCastsShadows(uint64_t aEntityID, bool aState);


		bool MeshRendererComponent_HasMaterial(uint64_t aEntityID, uint32_t aIndex);

		void MeshRendererComponent_AddMaterial(uint64_t aEntityID, AssetHandle* aHandle);

		void MeshRendererComponent_SetMaterial(uint64_t aEntityID, uint32_t aIndex, AssetHandle* aHandle);
		bool MeshRendererComponent_GetMaterial(uint64_t aEntityID, uint32_t aIndex, AssetHandle* outHandle);

#pragma endregion

#pragma region ScriptComponent
		
		MonoObject* ScriptComponent_GetInstance(uint64_t aEntityID);

#pragma endregion

#pragma region TextRendererComponent

		MonoString* TextComponent_GetText(uint64_t aEntityID);
		void TextComponent_SetText(uint64_t aEntityID, MonoString* aText);
		void TextComponent_GetColor(uint64_t aEntityID, CU::Color* outColor);
		void TextComponent_SetColor(uint64_t aEntityID, CU::Color* aColor);

#pragma endregion

#pragma region PointLightComponent

		void PointLightComponent_GetColor(uint64_t aEntityID, CU::Color* outColor);
		void PointLightComponent_SetColor(uint64_t aEntityID, CU::Color* aColor);

		float PointLightComponent_GetIntensity(uint64_t aEntityID);
		void PointLightComponent_SetIntensity(uint64_t aEntityID, float aIntensity);

		bool PointLightComponent_GetCastsShadows(uint64_t aEntityID);
		void PointLightComponent_SetCastsShadows(uint64_t aEntityID, bool aState);

		float PointLightComponent_GetRange(uint64_t aEntityID);
		void PointLightComponent_SetRange(uint64_t aEntityID, float aRange);

#pragma endregion
		
#pragma region SpotlightComponent

		void SpotlightComponent_GetColor(uint64_t aEntityID, CU::Color* outColor);
		void SpotlightComponent_SetColor(uint64_t aEntityID, CU::Color* aColor);

		float SpotlightComponent_GetIntensity(uint64_t aEntityID);
		void SpotlightComponent_SetIntensity(uint64_t aEntityID, float aIntensity);

		bool SpotlightComponent_GetCastsShadows(uint64_t aEntityID);
		void SpotlightComponent_SetCastsShadows(uint64_t aEntityID, bool aState);

		float SpotlightComponent_GetRange(uint64_t aEntityID);
		void SpotlightComponent_SetRange(uint64_t aEntityID, float aRange);

		float SpotlightComponent_GetOuterAngle(uint64_t aEntityID);
		void SpotlightComponent_SetOuterAngle(uint64_t aEntityID, float aAngle);

		float SpotlightComponent_GetInnerAngle(uint64_t aEntityID);
		void SpotlightComponent_SetInnerAngle(uint64_t aEntityID, float aAngle);

#pragma endregion

#pragma region Log

		enum class LogLevel : int32_t
		{
			Debug = 1u << 0,
			Info =	1u << 1,
			Warn =	1u << 2,
			Error = 1u << 3
		};


		void Log_LogMessage(LogLevel aLevel, MonoString* aInFormattedMessage);

#pragma endregion

#pragma region Gizmos
		
		void Gizmos_DrawWireSphere(CU::Vector3f* aCenter, float aRadius, CU::Color* aColor);
		void Gizmos_DrawWireCube(CU::Vector3f* aCenter, CU::Vector3f* aRotation, CU::Vector3f* aSize, CU::Color* aColor);
		void Gizmos_DrawLine(CU::Vector3f* aFrom, CU::Vector3f* aTo, CU::Color* aColor);

#pragma endregion

#pragma region Input

		bool Input_IsKeyPressed(KeyCode aKeyCode);
		bool Input_IsKeyHeld(KeyCode aKeyCode);
		bool Input_IsKeyReleased(KeyCode aKeyCode);
		
		bool Input_IsMouseButtonPressed(MouseButton aButton);
		bool Input_IsMouseButtonHeld(MouseButton aButton);
		bool Input_IsMouseButtonReleased(MouseButton aButton);

		void Input_GetMousePosition(CU::Vector2f* outPosition);
		void Input_GetMouseDelta(CU::Vector2f* outDelta);
		
		CursorMode Input_GetCursorMode();
		void Input_SetCursorMode(CursorMode aMode);
		
		void Input_GetScrollDelta(CU::Vector2f* outDelta);

		bool Input_IsGamepadButtonPressed(GamepadButton aButton);
		bool Input_IsGamepadButtonHeld(GamepadButton aButton);
		bool Input_IsGamepadButtonReleased(GamepadButton aButton);

		float Input_GetGamepadAxis(GamepadAxis aAxis);

#pragma endregion

#pragma region Physics

		bool Physics_Raycast(CU::Vector3f* aOrigin, CU::Vector3f* aDirection, float aMaxDistance, HitInfo* outHitInfo);

		bool Physics_SphereCast(CU::Vector3f* aOrigin, CU::Vector3f* aDirection, float aRadius, float aMaxDistance, HitInfo* outHitInfo);
		MonoArray* Physics_OverlapSphere(CU::Vector3f* aOrigin, float aRadius);

		void Physics_GetGravity(CU::Vector3f* outGravity);
		void Physics_SetGravity(CU::Vector3f* aGravity);

		void Physics_AddRadialImpulse(CU::Vector3f* aOrigin, float aRadius, float aStrength);

#pragma endregion
		
#pragma region RigidbodyComponent

		float RigidbodyComponent_GetMass(uint64_t aEntityID);
		void RigidbodyComponent_SetMass(uint64_t aEntityID, float aMass);

		bool RigidbodyComponent_GetUseGravity(uint64_t aEntityID);
		void RigidbodyComponent_SetUseGravity(uint64_t aEntityID, bool aState);
		
		float RigidbodyComponent_GetDrag(uint64_t aEntityID);
		void RigidbodyComponent_SetDrag(uint64_t aEntityID, float aDrag);
		
		float RigidbodyComponent_GetAngularDrag(uint64_t aEntityID);
		void RigidbodyComponent_SetAngularDrag(uint64_t aEntityID, float aDrag);
		
		void RigidbodyComponent_GetVelocity(uint64_t aEntityID, CU::Vector3f* outVelocity);
		void RigidbodyComponent_SetVelocity(uint64_t aEntityID, CU::Vector3f* aVelocity);
		
		void RigidbodyComponent_GetAngularVelocity(uint64_t aEntityID, CU::Vector3f* outVelocity);
		void RigidbodyComponent_SetAngularVelocity(uint64_t aEntityID, CU::Vector3f* aVelocity);
		
		void RigidbodyComponent_GetPosition(uint64_t aEntityID, CU::Vector3f* outPosition);
		void RigidbodyComponent_SetPosition(uint64_t aEntityID, CU::Vector3f* aPosition);
		
		void RigidbodyComponent_GetRotation(uint64_t aEntityID, CU::Vector3f* outRotation);
		void RigidbodyComponent_SetRotation(uint64_t aEntityID, CU::Vector3f* aRotation);
		
		Physics::Axis RigidbodyComponent_GetConstraints(uint64_t aEntityID);
		void RigidbodyComponent_SetConstraints(uint64_t aEntityID, Physics::Axis aConstraints);

		void RigidbodyComponent_AddForce(uint64_t aEntityID, CU::Vector3f* aForce, Physics::ForceMode aForceMode);
		void RigidbodyComponent_AddForceAtPosition(uint64_t aEntityID, CU::Vector3f* aForce, CU::Vector3f* aPosition, Physics::ForceMode aForceMode);

		void RigidbodyComponent_AddTorque(uint64_t aEntityID, CU::Vector3f* aTorque, Physics::ForceMode aForceMode);

		void RigidbodyComponent_Teleport(uint64_t aEntityID, CU::Vector3f* aTargetPosition, CU::Vector3f* aTargetRotation);
		
#pragma endregion

#pragma region CharacterControllerComponent

		float CharacterControllerComponent_GetStepOffset(uint64_t aEntityID);

		void CharacterControllerComponent_SetStepOffset(uint64_t aEntityID, float aStepOffset);

		float CharacterControllerComponent_GetSlopeLimit(uint64_t aEntityID);

		void CharacterControllerComponent_SetSlopeLimit(uint64_t aEntityID, float aSlopeLimit);

		void CharacterControllerComponent_Resize(uint64_t aEntityID, float aHeight);

		void CharacterControllerComponent_Move(uint64_t aEntityID, CU::Vector3f* aDisplacement);

#pragma endregion

#pragma region Texture2D

		bool Texture2D_Create(uint32_t aWidth, uint32_t aHeight, AssetHandle* outHandle);

		void Texture2D_GetSize(AssetHandle* aHandle, uint32_t* outWidth, uint32_t* outHeight);

		void Texture2D_SetData(AssetHandle* aHandle, MonoArray* aData);

#pragma endregion

#pragma region Material

		void Material_SetAlbedoTexture(AssetHandle* aMaterialHandle, AssetHandle* aTextureHandle);

#pragma endregion

#pragma region Mesh

		bool Mesh_Create(MonoArray* aVertexBuffer, MonoArray* aIndexBuffer, AssetHandle* outHandle);

#pragma endregion
	}
}
