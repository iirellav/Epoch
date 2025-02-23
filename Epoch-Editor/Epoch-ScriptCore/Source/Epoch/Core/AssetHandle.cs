using System.Runtime.InteropServices;

namespace Epoch
{
    [StructLayout(LayoutKind.Sequential)]
    public struct AssetHandle
    {
        public static readonly AssetHandle Invalid = new AssetHandle(0);

        internal ulong myHandle;

        public AssetHandle(ulong aHandle) { myHandle = aHandle; }

        public bool IsValid() => InternalCalls.AssetHandle_IsValid(ref this);

        public static implicit operator bool(AssetHandle assetHandle)
        {
            return InternalCalls.AssetHandle_IsValid(ref assetHandle);
        }

        public override string ToString() => myHandle.ToString();
        public override int GetHashCode() => myHandle.GetHashCode();
    }
}
