using System;
using System.Runtime.InteropServices;

namespace Epoch
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector2 : IEquatable<Vector2>
    {
        public float x;
        public float y;

        public static Vector2 Zero = new Vector2(0, 0);
        public static Vector2 One = new Vector2(1, 1);
        public static Vector2 Right = new Vector2(1, 0);
        public static Vector2 Left = new Vector2(-1, 0);
        public static Vector2 Up = new Vector2(0, 1);
        public static Vector2 Down = new Vector2(0, -1);

        public static Vector2 Infinity = new Vector2(float.PositiveInfinity);

        public Vector2(float aScalar) => x = y = aScalar;

        public Vector2(float aX, float aY)
        {
            x = aX;
            y = aY;
        }

        public Vector2(Vector3 aVec3)
        {
            x = aVec3.x;
            y = aVec3.y;
        }

        public override bool Equals(object aObj) => aObj is Vector2 aOther && Equals(aOther);
        public bool Equals(Vector2 aOther) => x == aOther.x && y == aOther.y;
        public override int GetHashCode() => (x, y).GetHashCode();

        public override string ToString() => $"Vector[{x}, {y}]";

        public static Vector2 operator *(Vector2 aVec0, float aScalar) => new Vector2(aVec0.x * aScalar, aVec0.y * aScalar);
        public static Vector2 operator *(float aScalar, Vector2 aVec1) => new Vector2(aScalar * aVec1.x, aScalar * aVec1.y);
        public static Vector2 operator *(Vector2 aVec0, Vector2 aVec1) => new Vector2(aVec0.x * aVec1.x, aVec0.y * aVec1.y);
        public static Vector2 operator /(Vector2 aVec0, Vector2 aVec1) => new Vector2(aVec0.x / aVec1.x, aVec0.y / aVec1.y);
        public static Vector2 operator /(Vector2 aVec0, float aScalar) => new Vector2(aVec0.x / aScalar, aVec0.y / aScalar);
        public static Vector2 operator /(float aScalar, Vector2 aVec1) => new Vector2(aScalar/ aVec1.x, aScalar/ aVec1.y);
        public static Vector2 operator +(Vector2 aVec0, Vector2 aVec1) => new Vector2(aVec0.x + aVec1.x, aVec0.y + aVec1.y);
        public static Vector2 operator +(Vector2 aVec0, float aScalar) => new Vector2(aVec0.x + aScalar, aVec0.y + aScalar);
        public static Vector2 operator -(Vector2 aVec0, Vector2 aVec1) => new Vector2(aVec0.x - aVec1.x, aVec0.y - aVec1.y);
        public static Vector2 operator -(Vector2 aVec0, float aScalar) => new Vector2(aVec0.x - aScalar, aVec0.y - aScalar);
        public static Vector2 operator -(Vector2 vector) => new Vector2(-vector.x, -vector.y);

        public static bool operator ==(Vector2 aVec0, Vector2 aVec1) => aVec0.Equals(aVec1);
        public static bool operator !=(Vector2 aVec0, Vector2 aVec1) => !(aVec0 == aVec1);

        public float Length() => (float)Math.Sqrt(x * x + y * y);
        public float LengthSqr() => (x * x + y * y);

        public Vector2 GetNormalized()
        {
            float length = Length();
            float invLength = 1.0f / length;
            return length > 0.0f ? new Vector2(x * invLength, y * invLength) : new Vector2(0, 0);
        }

        public void Normalize()
        {
            float length = Length();
            float invLength = 1.0f / length;
            if (length > 0.0f)
            {
                x *= invLength;
                y *= invLength;
            }
        }

        public static float Dot(Vector2 aVec0, Vector2 aVec1)
        {
            return (aVec0.x * aVec1.x) + (aVec0.y * aVec1.y);
        }

        public static float Distance(Vector2 aVec0, Vector2 aVec1)
        {
            return (aVec1 - aVec0).Length();
        }

        public static float DistanceSqr(Vector2 aVec0, Vector2 aVec1)
        {
            return (aVec1 - aVec0).LengthSqr();
        }

        public static Vector2 Lerp(Vector2 aFrom, Vector2 aTo, float aT)
        {
            return aFrom + (aTo - aFrom) * aT;
        }
    }
}
