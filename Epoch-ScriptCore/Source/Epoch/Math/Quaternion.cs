using System;
using System.Runtime.InteropServices;

namespace Epoch
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Quaternion : IEquatable<Quaternion>
    {
        public float w;
        public float x;
        public float y;
        public float z;

        public Quaternion(float aW, float aX, float aY, float aZ)
        {
            w = aW;
            x = aX;
            y = aY;
            z = aZ;
        }

        public Quaternion(float aW, Vector3 aXYZ)
        {
            w = aW;
            x = aXYZ.x;
            y = aXYZ.y;
            z = aXYZ.z;
        }

        public Quaternion(Vector3 aEuler)
        {
            float cp = Mathf.Cos(aEuler.x * 0.5f);
            float sp = Mathf.Sin(aEuler.x * 0.5f);
            float ch = Mathf.Cos(aEuler.y * 0.5f);
            float sh = Mathf.Sin(aEuler.y * 0.5f);
            float cb = Mathf.Cos(aEuler.z * 0.5f);
            float sb = Mathf.Sin(aEuler.z * 0.5f);

            w = ch * cp * cb + sh * sp * sb;
            x = ch * sp * cb + sh * cp * sb;
            y = sh * cp * cb - ch * sp * sb;
            z = ch * cp * sb - sh * sp * cb;
        }

        public Vector3 GetRight()
        {
            return new Vector3(
            1.0f - 2.0f * (y * y + z * z),
            2.0f * (x * y + w * z),
            2.0f * (x * z - w * y)
            ).GetNormalized();
        }

        public Vector3 GetUp()
        {
            return new Vector3(
            2.0f * (x * y - w * z),
            1.0f - 2.0f * (x * x + z * z),
            2.0f * (y * z + w * x)
            ).GetNormalized();
        }

        public Vector3 GetForward()
        {
            return new Vector3(
            2.0f * (x * z + w * y),
            2.0f * (y * z - w * x),
            1.0f - 2.0f * (x * x + y * y)
            ).GetNormalized();
        }

        public override int GetHashCode() => (w, x, y, z).GetHashCode();

        public override bool Equals(object aObj) => aObj is Quaternion aOther && Equals(aOther);
        public bool Equals(Quaternion aOther) => x == aOther.x && y == aOther.y && z == aOther.z && w == aOther.w;
    }
}
