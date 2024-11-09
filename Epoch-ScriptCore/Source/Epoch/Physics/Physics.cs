﻿using System.Runtime.InteropServices;

namespace Epoch
{
    public enum ForceMode
    {
        Force,              // A standard force, using Force = mass * distance / time^2
        Impulse,            // An Impulse force, using Force = mass * distance / time
        VelocityChange,     // An Impulse that ignores the objects mass, e.g Force = distance / time
        Acceleration        // A constant force, not accounting for mass, e.g Force = distance / time^2
    }

    public enum PhysicsAxis : byte
    {
        None = 0,

        TranslationX = 1 << 0,
        TranslationY = 1 << 1,
        TranslationZ = 1 << 2,
        Translation = TranslationX | TranslationY | TranslationZ,

        RotationX = 1 << 3,
        RotationY = 1 << 4,
        RotationZ = 1 << 5,
        Rotation = RotationX | RotationY | RotationZ
    }

    public static class Physics
    {
        public static Vector3 Gravity
        {
            get
            {
                InternalCalls.Physics_GetGravity(out Vector3 gravity);
                return gravity;
            }

            set => InternalCalls.Physics_SetGravity(ref value);
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct HitInfo
        {
            public ulong entityID { get; internal set; }
            public Vector3 point { get; internal set; }
            public Vector3 normal { get; internal set; }
            public float distance { get; internal set; }

            public Entity Entity => Scene.GetEntityByID(entityID);
        };

        public static bool Raycast(Vector3 aOrigin, Vector3 aDirection, float aMaxDistance, out HitInfo aHitInfo) => InternalCalls.Physics_Raycast(ref aOrigin, ref aDirection, aMaxDistance, out aHitInfo);

        public static void AddRadialImpulse(Vector3 aOrigin, float aRadius, float aStrength) => InternalCalls.Physics_AddRadialImpulse(ref aOrigin, aRadius, aStrength);
    }
}
