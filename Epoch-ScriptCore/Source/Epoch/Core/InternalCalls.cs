using System;
using System.Runtime.CompilerServices;

namespace Epoch
{
    internal static class InternalCalls
    {
        #region Application

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Application_Quit();

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Application_GetIsVSync();

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Application_SetIsVSync(bool aState);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern uint Application_GetWidth();

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern uint Application_GetHeight();

        #endregion

        #region Application

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Noise_SetSeed(int aSeed);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float Noise_SimplexNoise(float x, float y);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float Noise_PerlinNoise(float x, float y);

        #endregion

        #region Time

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float Time_GetTimeScale();

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Time_SetTimeScale(float aTimeScale);

        #endregion

        #region AssetHandle

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool AssetHandle_IsValid(ref AssetHandle aAssetHandle);

        #endregion

        #region SceneManager

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SceneManager_LoadScene(ref AssetHandle aAssetHandle);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern string SceneManager_GetCurrentSceneName();

        #endregion

        #region Scene

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Scene_IsEntityValid(ulong aEntityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern ulong Scene_GetEntityByName(string aName);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern ulong Scene_CreateEntity(string aName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern ulong Scene_DestroyEntity(ulong aEntityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern ulong Scene_DestroyAllChildren(ulong aEntityID);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern ulong Scene_InstantiatePrefab(ref AssetHandle aPrefabHandle);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern ulong Scene_InstantiatePrefabWithTranslation(ref AssetHandle aPrefabHandle, ref Vector3 aTranslation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern ulong Scene_InstantiatePrefabWithTranslationAndRotation(ref AssetHandle aPrefabHandle, ref Vector3 aTranslation, ref Vector3 aRotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern ulong Scene_InstantiatePrefabWithTransform(ref AssetHandle aPrefabHandle, ref Vector3 aTranslation, ref Vector3 aRotation, ref Vector3 aScale);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern ulong Scene_InstantiatePrefabWithParent(ref AssetHandle aPrefabHandle, ulong aParentId);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern ulong Scene_InstantiatePrefabWithTranslationWithParent(ref AssetHandle aPrefabHandle, ulong aParentId, ref Vector3 aTranslation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern ulong Scene_InstantiatePrefabWithTranslationAndRotationWithParent(ref AssetHandle aPrefabHandle, ulong aParentId, ref Vector3 aTranslation, ref Vector3 aRotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern ulong Scene_InstantiatePrefabWithTransformWithParent(ref AssetHandle aPrefabHandle, ulong aParentId, ref Vector3 aTranslation, ref Vector3 aRotation, ref Vector3 aScale);

        #endregion

        #region Entity

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Entity_GetIsActive(ulong aEntityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Entity_SetIsActive(ulong aEntityID, bool aState);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern ulong Entity_GetParent(ulong aEntityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Entity_SetParent(ulong aEntityID, ulong aParentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern Entity[] Entity_GetChildren(ulong aEntityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern ulong Entity_GetChildByName(ulong aParentID, string aName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Entity_AddComponent(ulong aEntityID, Type aType);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Entity_HasComponent(ulong aEntityID, Type aType);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Entity_RemoveComponent(ulong aEntityID, Type aType);

        #endregion

        #region TagComponent

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern string NameComponent_GetName(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void NameComponent_SetName(ulong entityID, string tag);

        #endregion

        #region TransformComponent

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_GetTransform(ulong aEntityID, out Transform outTransform);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_SetTransform(ulong aEntityID, ref Transform aTransform);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_GetWorldSpaceTransform(ulong aEntityID, out Transform outTransform);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_GetTranslation(ulong aEntityID, out Vector3 outTranslation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_GetRotation(ulong aEntityID, out Vector3 outRotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_GetScale(ulong aEntityID, out Vector3 outScale);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_SetTranslation(ulong aEntityID, ref Vector3 aTranslation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_SetRotation(ulong aEntityID, ref Vector3 aRotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_SetScale(ulong aEntityID, ref Vector3 aScale);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_Translate(ulong aEntityID, ref Vector3 aTranslation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_Rotate(ulong aEntityID, ref Vector3 aRotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_RotateAround(ulong aEntityID, ref Vector3 aPoint, ref Vector3 aAxis, float aAngle);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_LookAt(ulong aEntityID, ref Vector3 aTarget, ref Vector3 aUp);

        #endregion

        #region MeshRendererComponent

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool MeshRendererComponent_GetIsActive(ulong aEntityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void MeshRendererComponent_SetIsActive(ulong aEntityID, bool aState);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool MeshRendererComponent_GetMesh(ulong aEntityID, out AssetHandle outHandle);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void MeshRendererComponent_SetMesh(ulong aEntityID, ref AssetHandle aHandle);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool MeshRendererComponent_GetCastsShadows(ulong aEntityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void MeshRendererComponent_SetCastsShadows(ulong aEntityID, bool aState);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool MeshRendererComponent_HasMaterial(ulong aEntityID, uint aIndex);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void MeshRendererComponent_AddMaterial(ulong aEntityID, ref AssetHandle aHandle);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void MeshRendererComponent_SetMaterial(ulong aEntityID, uint aIndex, ref AssetHandle aHandle);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool MeshRendererComponent_GetMaterial(ulong aEntityID, uint aIndex, out AssetHandle outHandle);

        #endregion

        #region SpriteRendererComponent

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool SpriteRendererComponent_GetIsActive(ulong aEntityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SpriteRendererComponent_SetIsActive(ulong aEntityID, bool aState);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool SpriteRendererComponent_GetTexture(ulong aEntityID, out AssetHandle outHandle);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SpriteRendererComponent_SetTexture(ulong aEntityID, ref AssetHandle aHandle);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SpriteRendererComponent_GetTint(ulong aEntityID, out Color outColor);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SpriteRendererComponent_SetTint(ulong aEntityID, ref Color aColor);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool SpriteRendererComponent_GetFlipX(ulong aEntityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SpriteRendererComponent_SetFlipX(ulong aEntityID, bool aState);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool SpriteRendererComponent_GetFlipY(ulong aEntityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SpriteRendererComponent_SetFlipY(ulong aEntityID, bool aState);

        #endregion

        #region ScriptComponent

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern object ScriptComponent_GetInstance(ulong aEntityID);

        #endregion

        #region TextComponent

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern string TextComponent_GetText(ulong aEntityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TextComponent_SetText(ulong aEntityID, string aText);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TextComponent_GetColor(ulong aEntityID, out Color outColor);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TextComponent_SetColor(ulong aEntityID, ref Color aColor);

        #endregion

        #region PointLightComponent

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void PointLightComponent_GetColor(ulong aEntityID, out Color outColor);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void PointLightComponent_SetColor(ulong aEntityID, ref Color aColor);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float PointLightComponent_GetIntensity(ulong aEntityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void PointLightComponent_SetIntensity(ulong aEntityID, float aIntensity);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool PointLightComponent_GetCastsShadows(ulong aEntityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void PointLightComponent_SetCastsShadows(ulong aEntityID, bool aState);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float PointLightComponent_GetRange(ulong aEntityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void PointLightComponent_SetRange(ulong aEntityID, float aRange);

        #endregion

        #region SpotlightComponent

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SpotlightComponent_GetColor(ulong aEntityID, out Color outColor);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SpotlightComponent_SetColor(ulong aEntityID, ref Color aColor);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float SpotlightComponent_GetIntensity(ulong aEntityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SpotlightComponent_SetIntensity(ulong aEntityID, float aIntensity);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool SpotlightComponent_GetCastsShadows(ulong aEntityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SpotlightComponent_SetCastsShadows(ulong aEntityID, bool aState);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float SpotlightComponent_GetRange(ulong aEntityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SpotlightComponent_SetRange(ulong aEntityID, float aRange);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float SpotlightComponent_GetOuterAngle(ulong aEntityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SpotlightComponent_SetOuterAngle(ulong aEntityID, float aAngle);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float SpotlightComponent_GetInnerAngle(ulong aEntityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SpotlightComponent_SetInnerAngle(ulong aEntityID, float aAngle);

        #endregion

        #region Log

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Log_LogMessage(Log.LogLevel aLevel, string aFormattedMessage);

        #endregion

        #region Gizmos

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Gizmos_DrawWireSphere(ref Vector3 aCenter, float aRadius, ref Color aColor);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Gizmos_DrawWireCube(ref Vector3 aCenter, ref Vector3 aRotation, ref Vector3 aSize, ref Color aColor);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Gizmos_DrawLine(ref Vector3 aFrom, ref Vector3 aTo, ref Color aColor);

        #endregion

        #region Input

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Input_IsKeyPressed(KeyCode aKeyCode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Input_IsKeyHeld(KeyCode aKeyCode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Input_IsKeyReleased(KeyCode aKeyCode);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Input_IsMouseButtonPressed(MouseButton aButton);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Input_IsMouseButtonHeld(MouseButton aButton);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Input_IsMouseButtonReleased(MouseButton aButton);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Input_GetMousePosition(out Vector2 outMousePosition);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Input_GetMouseDelta(out Vector2 outMouseDelta);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern CursorMode Input_GetCursorMode();

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Input_SetCursorMode(CursorMode aMode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Input_GetScrollDelta(out Vector2 outScrollDelta);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Input_IsGamepadButtonPressed(GamepadButton aButton);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Input_IsGamepadButtonHeld(GamepadButton aButton);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Input_IsGamepadButtonReleased(GamepadButton aButton);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float Input_GetGamepadAxis(GamepadAxis aAxis);

        #endregion

        #region Physics

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Physics_Raycast(ref Vector3 aOrigin, ref Vector3 aDirection, float aMaxDistance, out Physics.HitInfo aHitInfo);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Physics_SphereCast(ref Vector3 aOrigin, ref Vector3 aDirection, float aRadius, float aMaxDistance, out Physics.HitInfo aHitInfo);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern Entity[] Physics_OverlapSphere(ref Vector3 aOrigin, float aRadius);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Physics_GetGravity(out Vector3 outGravity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Physics_SetGravity(ref Vector3 aGravity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Physics_AddRadialImpulse(ref Vector3 aOrigin, float aRadius, float aStrength);

        #endregion

        #region RigidBodyComponent

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float RigidbodyComponent_GetMass(ulong aEntityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void RigidbodyComponent_SetMass(ulong aEntityID, float aMass);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool RigidbodyComponent_GetUseGravity(ulong aEntityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void RigidbodyComponent_SetUseGravity(ulong aEntityID, bool aState);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float RigidbodyComponent_GetDrag(ulong aEntityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void RigidbodyComponent_SetDrag(ulong aEntityID, float aDrag);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float RigidbodyComponent_GetAngularDrag(ulong aEntityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void RigidbodyComponent_SetAngularDrag(ulong aEntityID, float aDrag);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void RigidbodyComponent_GetVelocity(ulong aEntityID, out Vector3 outVelocity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void RigidbodyComponent_SetVelocity(ulong aEntityID, ref Vector3 aVelocity);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void RigidbodyComponent_GetAngularVelocity(ulong aEntityID, out Vector3 outVelocity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void RigidbodyComponent_SetAngularVelocity(ulong aEntityID, ref Vector3 aVelocity);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void RigidbodyComponent_GetPosition(ulong aEntityID, out Vector3 outPosition);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void RigidbodyComponent_SetPosition(ulong aEntityID, ref Vector3 aPosition);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void RigidbodyComponent_GetRotation(ulong aEntityID, out Vector3 outRotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void RigidbodyComponent_SetRotation(ulong aEntityID, ref Vector3 aRotation);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern Physics.Axis RigidbodyComponent_GetConstraints(ulong aEntityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void RigidbodyComponent_SetConstraints(ulong aEntityID, Physics.Axis aConstraints);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void RigidbodyComponent_AddForce(ulong aEntityID, ref Vector3 aForce, Physics.ForceMode aForceMode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void RigidbodyComponent_AddForceAtPosition(ulong aEntityID, ref Vector3 aForce, ref Vector3 aPosition, Physics.ForceMode aForceMode);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void RigidbodyComponent_AddTorque(ulong aEntityID, ref Vector3 aTorque, Physics.ForceMode aForceMode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void RigidbodyComponent_Teleport(ulong aEntityID, ref Vector3 aTargetPosition, ref Vector3 aTargetRotation);

        #endregion

        #region CharacterControllerComponent

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float CharacterControllerComponent_GetStepOffset(ulong aEntityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void CharacterControllerComponent_SetStepOffset(ulong aEntityID, float aStepOffset);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float CharacterControllerComponent_GetSlopeLimit(ulong aEntityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void CharacterControllerComponent_SetSlopeLimit(ulong aEntityID, float aSlopeLimit);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void CharacterControllerComponent_Resize(ulong aEntityID, float aHeight);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void CharacterControllerComponent_Move(ulong aEntityID, ref Vector3 aDisplacement);

        #endregion

        #region Texture2D

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Texture2D_Create(uint aWidth, uint aHeight/*, ref string aName*/, out AssetHandle outHandle);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Texture2D_GetSize(ref AssetHandle aHandle, out uint outWidth, out uint outHeight);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Texture2D_SetData(ref AssetHandle aHandle, Color[] aData);

        #endregion

        #region Material

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Material_SetAlbedoTexture(ref AssetHandle aMaterialHandle, ref AssetHandle aTextureHandle);

        #endregion

        #region Mesh

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Mesh_Create(Vertex[] aVertexBuffer, uint[] aIndexBuffer, out AssetHandle outHandle);

        #endregion
    }
}
