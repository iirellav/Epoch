using System;

namespace Epoch
{
    public class Prefab : IEquatable<Prefab>
    {
        internal AssetHandle myHandle;
        public AssetHandle Handle => myHandle;

        internal Prefab() { myHandle = AssetHandle.Invalid; }
        internal Prefab(AssetHandle aHandle) { myHandle = aHandle; }

        public static implicit operator bool(Prefab prefab)
        {
            return prefab.myHandle;
        }

        public override bool Equals(object aObj) => aObj is Prefab aOther && Equals(aOther);

        public bool Equals(Prefab aOther)
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

        public static bool operator ==(Prefab aPrefabA, Prefab aPrefabB) => aPrefabA is null ? aPrefabB is null : aPrefabA.Equals(aPrefabB);
        public static bool operator !=(Prefab aPrefabA, Prefab aPrefabB) => !(aPrefabA == aPrefabB);
    }
}
