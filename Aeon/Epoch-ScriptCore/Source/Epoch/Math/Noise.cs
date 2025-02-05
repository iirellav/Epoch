using System;

namespace Epoch
{
    public static class Noise
    {
        public static void SetSeed(int aSeed) => InternalCalls.Noise_SetSeed(aSeed);
        public static float SimplexNoise(float x, float y) => InternalCalls.Noise_SimplexNoise(x, y);
        public static float PerlinNoise(float x, float y) => InternalCalls.Noise_PerlinNoise(x, y);
    }
}
