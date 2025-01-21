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
    }
}
