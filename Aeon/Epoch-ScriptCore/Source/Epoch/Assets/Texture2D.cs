using System;

namespace Epoch
{
    public class Texture2D : IEquatable<Texture2D>
    {
        internal AssetHandle myHandle;
        public AssetHandle Handle => myHandle;
        public uint Width { get; private set; }
        public uint Height { get; private set; }

        internal Texture2D() { myHandle = AssetHandle.Invalid; }
        internal Texture2D(AssetHandle aHandle)
        {
            myHandle = aHandle;
            InternalCalls.Texture2D_GetSize(ref myHandle, out uint outWidth, out uint outHeight);
            Width = outWidth;
            Height = outHeight;
        }

        public override bool Equals(object aObj) => aObj is Texture2D aOther && Equals(aOther);

        public bool Equals(Texture2D aOther)
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

        public static bool operator ==(Texture2D aLeft, Texture2D aRight) => aLeft is null ? aRight is null : aLeft.Equals(aRight);
        public static bool operator !=(Texture2D aLeft, Texture2D aRight) => !(aLeft == aRight);

        public void SetData(Color[] aData) => InternalCalls.Texture2D_SetData(ref myHandle, aData);

        public static Texture2D Create(uint aWidth, uint aHeight, Color[] aData = null)
        {
            if (aWidth == 0)
            {
                throw new ArgumentException("Tried to create a Texture2D with a width of 0.");
            }

            if (aHeight == 0)
            {
                throw new ArgumentException("Tried to create a Texture2D with a height of 0.");
            }

            if (!InternalCalls.Texture2D_Create(aWidth, aHeight, out AssetHandle handle))
            {
                return null;
            }

            Texture2D texture = new Texture2D() { myHandle = handle, Width = aWidth, Height = aHeight };

            if (aData != null && aData.Length > 0)
            {
                texture.SetData(aData);
            }

            return texture;
        }
    }
}
