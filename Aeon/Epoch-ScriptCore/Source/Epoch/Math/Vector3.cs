using System;
using System.Runtime.InteropServices;

namespace Epoch
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector3 : IEquatable<Vector3>
    {
        public float x;
        public float y;
        public float z;

        public static Vector3 Zero =        new Vector3(0, 0, 0);
        public static Vector3 One =         new Vector3(1, 1, 1);
        public static Vector3 Forward =     new Vector3(0, 0, 1);
        public static Vector3 Backward =    new Vector3(0, 0, -1);
        public static Vector3 Right =       new Vector3(1, 0, 0);
        public static Vector3 Left =        new Vector3(-1, 0, 0);
        public static Vector3 Up =          new Vector3(0, 1, 0);
        public static Vector3 Down =        new Vector3(0, -1, 0);

        public static Vector3 Infinity = new Vector3(float.PositiveInfinity);

        public Vector3(float aScalar) => x = y = z = aScalar;

        public Vector3(float aX, float aY, float aZ)
        {
            x = aX;
            y = aY;
            z = aZ;
        }

        public Vector3(Vector2 aVec2)
        {
            x = aVec2.x;
            y = aVec2.y;
            z = 0;
        }

        public override bool Equals(object aObj) => aObj is Vector3 aOther && Equals(aOther);
        public bool Equals(Vector3 aOther) => x == aOther.x && y == aOther.y && z == aOther.z;
        public override int GetHashCode() => (x, y, z).GetHashCode();

        public override string ToString() => $"Vector3[{x}, {y}, {z}]";

        public static Vector3 operator *(Vector3 aVec0, float aScalar) => new Vector3(aVec0.x * aScalar, aVec0.y * aScalar, aVec0.z * aScalar);
        public static Vector3 operator *(float aScalar, Vector3 aVec1) => new Vector3(aScalar * aVec1.x, aScalar * aVec1.y, aScalar * aVec1.z);
        public static Vector3 operator *(Vector3 aVec0, Vector3 aVec1) => new Vector3(aVec0.x * aVec1.x, aVec0.y * aVec1.y, aVec0.z * aVec1.z);
        public static Vector3 operator /(Vector3 aVec0, Vector3 aVec1) => new Vector3(aVec0.x / aVec1.x, aVec0.y / aVec1.y, aVec0.z / aVec1.z);
        public static Vector3 operator /(Vector3 aVec0, float aScalar) => new Vector3(aVec0.x / aScalar, aVec0.y / aScalar, aVec0.z / aScalar);
        public static Vector3 operator /(float aScalar, Vector3 aVec1) => new Vector3(aScalar/ aVec1.x, aScalar/ aVec1.y, aScalar/ aVec1.z);
        public static Vector3 operator +(Vector3 aVec0, Vector3 aVec1) => new Vector3(aVec0.x + aVec1.x, aVec0.y + aVec1.y, aVec0.z + aVec1.z);
        public static Vector3 operator +(Vector3 aVec0, float aScalar) => new Vector3(aVec0.x + aScalar, aVec0.y + aScalar, aVec0.z + aScalar);
        public static Vector3 operator -(Vector3 aVec0, Vector3 aVec1) => new Vector3(aVec0.x - aVec1.x, aVec0.y - aVec1.y, aVec0.z - aVec1.z);
        public static Vector3 operator -(Vector3 aVec0, float aScalar) => new Vector3(aVec0.x - aScalar, aVec0.y - aScalar, aVec0.z - aScalar);
        public static Vector3 operator -(Vector3 vector) => new Vector3(-vector.x, -vector.y, -vector.z);

        public static bool operator ==(Vector3 aVec0, Vector3 aVec1) => aVec0.Equals(aVec1);
        public static bool operator !=(Vector3 aVec0, Vector3 aVec1) => !(aVec0 == aVec1);

        public float Length() => (float)Math.Sqrt(x * x + y * y + z * z);
        public float LengthSqr() => (x * x + y * y + z * z);

        public Vector3 GetNormalized()
        {
            float length = Length();
            float invLength = 1.0f / length;
            return length > 0.0f ? new Vector3(x * invLength, y * invLength, z * invLength) : new Vector3(0, 0, 0);
        }

        public void Normalize()
        {
            float length = Length();
            float invLength = 1.0f / length;
            if (length > 0.0f)
            {
                x *= invLength;
                y *= invLength;
                z *= invLength;
            }
        }

        public static float Dot(Vector3 aVec0, Vector3 aVec1)
        {
            return (aVec0.x * aVec1.x) + (aVec0.y * aVec1.y) + (aVec0.z * aVec1.z);
        }

        public static Vector3 Cross(Vector3 aVec0, Vector3 aVec1)
        {
            return new Vector3(
                (aVec0.y * aVec1.z) - (aVec0.z * aVec1.y),
                (aVec0.z * aVec1.x) - (aVec0.x * aVec1.z),
                (aVec0.x * aVec1.y) - (aVec0.y * aVec1.x)
            );
        }

        public static float Distance(Vector3 aVec0, Vector3 aVec1)
        {
            return (aVec1 - aVec0).Length();
        }

        public static float DistanceSqr(Vector3 aVec0, Vector3 aVec1)
        {
            return (aVec1 - aVec0).LengthSqr();
        }

        public static Vector3 Lerp(Vector3 aFrom, Vector3 aTo, float aT)
        {
            return aFrom + (aTo - aFrom) * aT;
        }
    }
}
