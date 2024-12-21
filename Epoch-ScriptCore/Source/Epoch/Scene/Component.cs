using System;
using System.Reflection;
using System.Security.Policy;
using System.Threading;

namespace Epoch
{
    public abstract class Component
    {
        public Entity entity { get; internal set; }
    }

    public class NameComponent : Component
    {
        public string Tag
        {
            get => InternalCalls.NameComponent_GetName(entity.id);
            set => InternalCalls.NameComponent_SetName(entity.id, value);
        }
    }

    public class TransformComponent : Component
    {
        /// <summary>
		/// Transform relative to parent entity
		/// </summary>
		public Transform LocalTransform
        {
            get
            {
                InternalCalls.TransformComponent_GetTransform(entity.id, out Transform result);
                return result;
            }

            set => InternalCalls.TransformComponent_SetTransform(entity.id, ref value);
        }

        /// <summary>
        /// Transform in world coordinate space
        /// </summary>
        public Transform WorldTransform
        {
            get
            {
                InternalCalls.TransformComponent_GetWorldSpaceTransform(entity.id, out Transform result);
                return result;
            }
        }

        public Vector3 Position
        {
            get
            {
                InternalCalls.TransformComponent_GetTranslation(entity.id, out Vector3 result);
                return result;
            }

            set => InternalCalls.TransformComponent_SetTranslation(entity.id, ref value);
        }

        public Vector3 Rotation
        {
            get
            {
                InternalCalls.TransformComponent_GetRotation(entity.id, out Vector3 result);
                return result;
            }

            set => InternalCalls.TransformComponent_SetRotation(entity.id, ref value);
        }

        public Vector3 Scale
        {
            get
            {
                InternalCalls.TransformComponent_GetScale(entity.id, out Vector3 result);
                return result;
            }

            set => InternalCalls.TransformComponent_SetScale(entity.id, ref value);
        }

        public void Translate(Vector3 aTranslation)
        {
            InternalCalls.TransformComponent_Translate(entity.id, ref aTranslation);
        }

        public void Rotate(Vector3 aRotation)
        {
            InternalCalls.TransformComponent_Rotate(entity.id, ref aRotation);
        }

        public void RotateAround(Vector3 aPoint, Vector3 aAxis, float aAngle)
        {
            InternalCalls.TransformComponent_RotateAround(entity.id, ref aPoint, ref aAxis, aAngle);
        }

        public void LookAt(Vector3 aTarget)
        {
            LookAt(aTarget, Vector3.Up);
        }

        public void LookAt(Vector3 aTarget, Vector3 aUp)
        {
            InternalCalls.TransformComponent_LookAt(entity.id, ref aTarget, ref aUp);
        }
    }

    public class MeshRendererComponent : Component
    {
        public bool Active
        {
            get => InternalCalls.MeshRendererComponent_GetIsActive(entity.id);
            set => InternalCalls.MeshRendererComponent_SetIsActive(entity.id, value);
        }

        public Mesh Mesh
        {
            get
            {
                if (!InternalCalls.MeshRendererComponent_GetMesh(entity.id, out AssetHandle outMeshHandle))
                {
                    return null;
                }

                return new Mesh(outMeshHandle);
            }

            set => InternalCalls.MeshRendererComponent_SetMesh(entity.id, ref value.myHandle);
        }

        public bool CastsShadows
        {
            get => InternalCalls.MeshRendererComponent_GetCastsShadows(entity.id);
            set => InternalCalls.MeshRendererComponent_SetCastsShadows(entity.id, value);
        }

        public bool HasMaterial(uint aIndex) => InternalCalls.MeshRendererComponent_HasMaterial(entity.id, aIndex);

        public Material GetMaterial(uint aIndex = 0)
        {
            if (!HasMaterial(aIndex) || !InternalCalls.MeshRendererComponent_GetMaterial(entity.id, aIndex, out AssetHandle outMaterialHandle))
            {
                return null;
            }

            return new Material(outMaterialHandle);
        }

        public void SetMaterial(Material aMaterial)
        {
            if (!HasMaterial(0))
            {
                InternalCalls.MeshRendererComponent_AddMaterial(entity.id, ref aMaterial.myHandle);
            }
            else
            {
                SetMaterial(0, aMaterial);
            }
        }

        public void SetMaterial(uint aIndex, Material aMaterial)
        {
            if (!HasMaterial(aIndex))
            {
                return;
            }

            InternalCalls.MeshRendererComponent_SetMaterial(entity.id, aIndex, ref aMaterial.myHandle);
        }
    }

    public class SpriteRendererComponent : Component
    {
        public bool Active
        {
            get => InternalCalls.SpriteRendererComponent_GetIsActive(entity.id);
            set => InternalCalls.SpriteRendererComponent_SetIsActive(entity.id, value);
        }

        public Texture2D Texture
        {
            get
            {
                if (!InternalCalls.SpriteRendererComponent_GetTexture(entity.id, out AssetHandle outTextureHandle))
                {
                    return null;
                }

                return new Texture2D(outTextureHandle);
            }

            set => InternalCalls.SpriteRendererComponent_SetTexture(entity.id, ref value.myHandle);
        }

        public Color Color
        {
            get
            {
                InternalCalls.SpriteRendererComponent_GetTint(entity.id, out Color color);
                return color;
            }

            set => InternalCalls.SpriteRendererComponent_SetTint(entity.id, ref value);
        }

        public bool FlipX
        {
            get => InternalCalls.SpriteRendererComponent_GetFlipX(entity.id);
            set => InternalCalls.SpriteRendererComponent_SetFlipX(entity.id, value);
        }

        public bool FlipY
        {
            get => InternalCalls.SpriteRendererComponent_GetFlipY(entity.id);
            set => InternalCalls.SpriteRendererComponent_SetFlipY(entity.id, value);
        }
    }

    public class TextRendererComponent : Component
    {
        public string Text
        {
            get
            {
                return InternalCalls.TextComponent_GetText(entity.id);
            }

            set => InternalCalls.TextComponent_SetText(entity.id, value);
        }

        public Color Color
        {
            get
            {
                InternalCalls.TextComponent_GetColor(entity.id, out Color color);
                return color;
            }

            set => InternalCalls.TextComponent_SetColor(entity.id, ref value);
        }
    }

    public class PointLightComponent : Component
    {
        public Color Color
        {
            get
            {
                InternalCalls.PointLightComponent_GetColor(entity.id, out Color color);
                return color;
            }

            set => InternalCalls.PointLightComponent_SetColor(entity.id, ref value);
        }

        public float Intensity
        {
            get => InternalCalls.PointLightComponent_GetIntensity(entity.id);
            set => InternalCalls.PointLightComponent_SetIntensity(entity.id, value);
        }

        public bool CastsShadows
        {
            get => InternalCalls.PointLightComponent_GetCastsShadows(entity.id);
            set => InternalCalls.PointLightComponent_SetCastsShadows(entity.id, value);
        }

        public float Range
        {
            get => InternalCalls.PointLightComponent_GetRange(entity.id);
            set => InternalCalls.PointLightComponent_SetRange(entity.id, value);
        }
    }

    public class SpotlightComponent : Component
    {
        public Color Color
        {
            get
            {
                InternalCalls.SpotlightComponent_GetColor(entity.id, out Color color);
                return color;
            }

            set => InternalCalls.SpotlightComponent_SetColor(entity.id, ref value);
        }

        public float Intensity
        {
            get => InternalCalls.SpotlightComponent_GetIntensity(entity.id);
            set => InternalCalls.SpotlightComponent_SetIntensity(entity.id, value);
        }

        public bool CastsShadows
        {
            get => InternalCalls.SpotlightComponent_GetCastsShadows(entity.id);
            set => InternalCalls.SpotlightComponent_SetCastsShadows(entity.id, value);
        }

        public float Range
        {
            get => InternalCalls.SpotlightComponent_GetRange(entity.id);
            set => InternalCalls.SpotlightComponent_SetRange(entity.id, value);
        }

        public float OuterAngle
        {
            get => InternalCalls.SpotlightComponent_GetOuterAngle(entity.id);
            set => InternalCalls.SpotlightComponent_SetOuterAngle(entity.id, value);
        }

        public float InnerAngle
        {
            get => InternalCalls.SpotlightComponent_GetInnerAngle(entity.id);
            set => InternalCalls.SpotlightComponent_SetInnerAngle(entity.id, value);
        }
    }

    public class ScriptComponent : Component
    {
        public object Instance => InternalCalls.ScriptComponent_GetInstance(entity.id);
    }

    public class RigidbodyComponent : Component
    {
        public float Mass
        {
            get => InternalCalls.RigidbodyComponent_GetMass(entity.id);
            set => InternalCalls.RigidbodyComponent_SetMass(entity.id, value);
        }

        public bool UseGravity
        {
            get => InternalCalls.RigidbodyComponent_GetUseGravity(entity.id);
            set => InternalCalls.RigidbodyComponent_SetUseGravity(entity.id, value);
        }

        public float Drag
        {
            get => InternalCalls.RigidbodyComponent_GetDrag(entity.id);
            set => InternalCalls.RigidbodyComponent_SetDrag(entity.id, value);
        }

        public float AngularDrag
        {
            get => InternalCalls.RigidbodyComponent_GetAngularDrag(entity.id);
            set => InternalCalls.RigidbodyComponent_SetAngularDrag(entity.id, value);
        }

        public Vector3 Velocity
        {
            get
            {
                InternalCalls.RigidbodyComponent_GetVelocity(entity.id, out Vector3 result);
                return result;
            }

            set => InternalCalls.RigidbodyComponent_SetVelocity(entity.id, ref value);
        }

        public Vector3 AngularVelocity
        {
            get
            {
                InternalCalls.RigidbodyComponent_GetAngularVelocity(entity.id, out Vector3 result);
                return result;
            }

            set => InternalCalls.RigidbodyComponent_SetAngularVelocity(entity.id, ref value);
        }

        public Vector3 Position
        {
            get
            {
                InternalCalls.RigidbodyComponent_GetPosition(entity.id, out Vector3 result);
                return result;
            }

            set => InternalCalls.RigidbodyComponent_SetPosition(entity.id, ref value);
        }

        public Vector3 Rotation
        {
            get
            {
                InternalCalls.RigidbodyComponent_GetRotation(entity.id, out Vector3 result);
                return result;
            }

            set => InternalCalls.RigidbodyComponent_SetRotation(entity.id, ref value);
        }

        public Physics.Axis Constraints
        {
            get => InternalCalls.RigidbodyComponent_GetConstraints(entity.id);
            set => InternalCalls.RigidbodyComponent_SetConstraints(entity.id, value);
        }

        public void AddForce(Vector3 aForce, Physics.ForceMode aForceMode = Physics.ForceMode.Force) => InternalCalls.RigidbodyComponent_AddForce(entity.id, ref aForce, aForceMode);
        public void AddForceAtPosition(Vector3 aForce, Vector3 aPosition, Physics.ForceMode aForceMode = Physics.ForceMode.Force) => InternalCalls.RigidbodyComponent_AddForceAtPosition(entity.id, ref aForce, ref aPosition, aForceMode);

        public void AddTorque(Vector3 aTorque, Physics.ForceMode aForceMode = Physics.ForceMode.Force) => InternalCalls.RigidbodyComponent_AddTorque(entity.id, ref aTorque, aForceMode);

        public void Teleport(Vector3 aTargetPosition, Vector3 aTargetRotation) => InternalCalls.RigidbodyComponent_Teleport(entity.id, ref aTargetPosition, ref aTargetRotation);
    }

    public class CharacterControllerComponent : Component
    {
        public float StepOffset
        {
            get => InternalCalls.CharacterControllerComponent_GetStepOffset(entity.id);
            set => InternalCalls.CharacterControllerComponent_SetStepOffset(entity.id, value);
        }

        public float SlopeLimit
        {
            get => InternalCalls.CharacterControllerComponent_GetSlopeLimit(entity.id);
            set => InternalCalls.CharacterControllerComponent_SetSlopeLimit(entity.id, value);
        }

        void Resize(float aHeight) => InternalCalls.CharacterControllerComponent_Resize(entity.id, aHeight);

        public void Move(Vector3 aDisplacement) => InternalCalls.CharacterControllerComponent_Move(entity.id, ref aDisplacement);
    }
}
