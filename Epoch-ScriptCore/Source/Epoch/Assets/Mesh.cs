using System;

namespace Epoch
{
    public class Mesh : IEquatable<Mesh>
    {
        internal AssetHandle myHandle;
        public AssetHandle Handle => myHandle;

        internal Mesh() { myHandle = AssetHandle.Invalid; }
        internal Mesh(AssetHandle aHandle) { myHandle = aHandle; }

        public override bool Equals(object aObj) => aObj is Mesh aOther && Equals(aOther);

        public bool Equals(Mesh aOther)
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

        public static bool operator ==(Mesh aLeft, Mesh aRight) => aLeft is null ? aRight is null : aLeft.Equals(aRight);
        public static bool operator !=(Mesh aLeft, Mesh aRight) => !(aLeft == aRight);
    }
}
