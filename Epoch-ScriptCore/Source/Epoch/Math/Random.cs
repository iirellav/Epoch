using System;

namespace Epoch
{
    public static class Random
    {
        private static ulong[] staticState;
        private static System.Random staticRand;

        private const double INCR_DOUBLE = 1.0 / (1UL << 53);
        private const float INCR_FLOAT = 1f / (1U << 24);

        static Random()
        {
            staticRand = new System.Random();
        }

        public static int UInt32()
        {
            return staticRand.Next();
        }

        public static float Float()
        {
            return (UInt32() >> 40) * INCR_FLOAT;
        }

        public static Vector3 Vec3()
        {
            return new Vector3(Float(), Float(), Float());
        }

        public static double Double()
        {
            return (UInt32() >> 11) * INCR_DOUBLE;
        }

        public static float SignF()
        {
            return UInt32() % 2 == 0 ? 1.0f : -1.0f;
        }

        public static float Range(float minValue, float maxValue)
        {
            return Float() * (maxValue - minValue) + minValue;
        }

        public static int Range(int minValue, int maxValue)
        {
            return ((int)(UInt32()>>33) % (maxValue - minValue)) + minValue;
        }
    }
}
