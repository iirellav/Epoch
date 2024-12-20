using System;
using System.Runtime.InteropServices;

namespace Epoch
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vertex
    {
        public Vector3 position;
        public Vector3 normal;
        public Vector3 tangent;
        public Vector2 uv;
        public Vector3 color;
    }

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

        public static Mesh Create(Vertex[] aVertexBuffer, uint[] aIndexBuffer)
        {
            if (aVertexBuffer == null || aVertexBuffer.Length == 0)
            {
                throw new ArgumentException("Tried to create a Mesh with an empty vertex buffer");
            }

            if (aIndexBuffer == null || aIndexBuffer.Length == 0)
            {
                throw new ArgumentException("Tried to create a Mesh with an empty index buffer");
            }

            if (!InternalCalls.Mesh_Create(aVertexBuffer, aIndexBuffer, out AssetHandle outHandle))
            {
                return null;
            }

            Mesh mesh = new Mesh(outHandle);
            return mesh;
        }
    }
}
