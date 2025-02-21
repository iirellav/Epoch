using System;

namespace Epoch
{
    public sealed class Material : IEquatable<Material>
    {
        internal AssetHandle myHandle;
        public AssetHandle Handle => myHandle;

        internal Material() { myHandle = AssetHandle.Invalid; }
        internal Material(AssetHandle aHandle) { myHandle = aHandle; }

        public override bool Equals(object aObj) => aObj is Material aOther && Equals(aOther);

        public bool Equals(Material aOther)
        {
            if (aOther is null)
            {
                return false;
            }

            if (ReferenceEquals(this, aOther))
            {
                return true;
            }

            return myHandle == aOther.myHandle;
        }

        public override int GetHashCode() => myHandle.GetHashCode();

        public static bool operator ==(Material aLeft, Material aRight) => aLeft is null ? aRight is null : aLeft.Equals(aRight);
        public static bool operator !=(Material aLeft, Material aRight) => !(aLeft == aRight);

        public void SetAlbedoTexture(Texture2D aTexture) => InternalCalls.Material_SetAlbedoTexture(ref myHandle, ref aTexture.myHandle);

        public Color AlbedoColor
        {
            get
            {
                InternalCalls.Material_GetAlbedoColor(ref myHandle, out Color color);
                return color;
            }

            set => InternalCalls.Material_SetAlbedoColor(ref myHandle, ref value);
        }

        public Color EmissionColor
        {
            get
            {
                InternalCalls.Material_GetEmissionColor(ref myHandle, out Color color);
                return color;
            }

            set => InternalCalls.Material_SetEmissionColor(ref myHandle, ref value);
        }

        public float EmissionStrength
        {
            get => InternalCalls.Material_GetEmissionStrength(ref myHandle);
            set => InternalCalls.Material_SetEmissionStrength(ref myHandle, value);
        }

        public float NormalStrength
        {
            get => InternalCalls.Material_GetNormalStrength(ref myHandle);
            set => InternalCalls.Material_SetNormalStrength(ref myHandle, value);
        }

        public float Roughness
        {
            get => InternalCalls.Material_GetRoughness(ref myHandle);
            set => InternalCalls.Material_SetRoughness(ref myHandle, value);
        }

        public float Metalness
        {
            get => InternalCalls.Material_GetMetalness(ref myHandle);
            set => InternalCalls.Material_SetMetalness(ref myHandle, value);
        }

        public Vector2 UVTiling
        {
            get
            {
                InternalCalls.Material_GetUVTiling(ref myHandle, out Vector2 tiling);
                return tiling;
            }

            set => InternalCalls.Material_SetUVTiling(ref myHandle, ref value);
        }

        public Vector2 UVOffset
        {
            get
            {
                InternalCalls.Material_GetUVOffset(ref myHandle, out Vector2 offset);
                return offset;
            }

            set => InternalCalls.Material_SetUVOffset(ref myHandle, ref value);
        }
    }
}
