using System;

namespace Epoch
{
    public static class Mathf
    {
        public const float Epsilon = 0.000001f;
        public const float Infinity = 340282300000.0f;
        public const float PI = (float)Math.PI;

        public const float ToRad = PI / 180.0f;
        public const float ToDeg = 180.0f / PI;


        public static float Sqrt(float aValue) => (float)Math.Sqrt(aValue);
        
        public static float Sin(float aValue) => (float)Math.Sin(aValue);
        public static float Cos(float aValue) => (float)Math.Cos(aValue);
        public static float Tan(float aValue) => (float)Math.Tan(aValue);

        public static float Clamp(float value, float min, float max)
        {
            return value > max ? max : value < min ? min : value;
        }

        public static int CeilToInt(float aValue) => (int)Math.Ceiling(aValue);
        public static int FloorToInt(float aValue) => (int)Math.Floor(aValue);

        public static float Lerp(float aFrom, float aTo, float aT)
        {
            return aFrom + (aTo - aFrom) * aT;
        }
    }
}
