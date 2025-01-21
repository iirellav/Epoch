using System;

namespace Epoch
{
    public struct Time
    {
        public static float DeltaTime { get; private set; }
        public static float UnscaledDeltaTime { get; private set; }
        public static float FixedDeltaTime { get; private set; }

        private static void UpdateDeltaTime(float aNewDeltaTime) => DeltaTime = aNewDeltaTime;
        private static void UpdateUnscaledDeltaTime(float aNewDeltaTime) => UnscaledDeltaTime = aNewDeltaTime;
        private static void UpdateFixedDeltaTime(float aNewFixedDeltaTime) => FixedDeltaTime = aNewFixedDeltaTime;

        public static float GetTimeScale() => InternalCalls.Time_GetTimeScale();
        public static void SetTimeScale(float aTimeScale) => InternalCalls.Time_SetTimeScale(aTimeScale);
    }
}
