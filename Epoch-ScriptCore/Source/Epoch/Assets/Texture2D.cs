using System;

namespace Epoch
{
    public class Texture2D
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

        public void SetData(Color[] aData) => InternalCalls.Texture2D_SetData(ref myHandle, aData);

        public static Texture2D Create(uint aWidth, uint aHeight, Color[] aData = null/*, string aName = null*/)
        {
            if (aWidth == 0)
            {
                throw new ArgumentException("Tried to create a Texture2D with a width of 0.");
            }

            if (aHeight == 0)
            {
                throw new ArgumentException("Tried to create a Texture2D with a height of 0.");
            }

            if (!InternalCalls.Texture2D_Create(aWidth, aHeight/*, ref aName*/, out AssetHandle handle))
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
