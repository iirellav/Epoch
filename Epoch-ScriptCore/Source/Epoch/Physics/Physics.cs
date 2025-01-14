using System;
using System.Runtime.InteropServices;

namespace Epoch
{
    [StructLayout(LayoutKind.Sequential)]
    public struct LayerMask
    {
        public UInt32 bitValue;
        public LayerMask(UInt32 aValue = 1u) { bitValue = aValue; }

        public override string ToString() => bitValue.ToString();
        public override int GetHashCode() => bitValue.GetHashCode();
    };

    public static class Physics
    {
        public enum ForceMode
        {
            Force,              // A standard force, using Force = mass * distance / time^2
            Impulse,            // An Impulse force, using Force = mass * distance / time
            VelocityChange,     // An Impulse that ignores the objects mass, e.g Force = distance / time
            Acceleration        // A constant force, not accounting for mass, e.g Force = distance / time^2
        }

        public enum Axis : byte
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


        public static bool Raycast(Vector3 aOrigin, Vector3 aDirection, float aMaxDistance = Mathf.Infinity) => InternalCalls.Physics_Raycast(ref aOrigin, ref aDirection, aMaxDistance, out _);

        public static bool Raycast(Vector3 aOrigin, Vector3 aDirection, LayerMask aLayerMask, float aMaxDistance = Mathf.Infinity) => InternalCalls.Physics_RaycastFiltered(ref aOrigin, ref aDirection, aMaxDistance, out _, ref aLayerMask);

        public static bool Raycast(Vector3 aOrigin, Vector3 aDirection, out HitInfo outHitInfo, float aMaxDistance = Mathf.Infinity) => InternalCalls.Physics_Raycast(ref aOrigin, ref aDirection, aMaxDistance, out outHitInfo);

        public static bool Raycast(Vector3 aOrigin, Vector3 aDirection, out HitInfo outHitInfo, LayerMask aLayerMask, float aMaxDistance = Mathf.Infinity) => InternalCalls.Physics_RaycastFiltered(ref aOrigin, ref aDirection, aMaxDistance, out outHitInfo, ref aLayerMask);


        public static bool SphereCast(Vector3 aOrigin, Vector3 aDirection, float aRadius, float aMaxDistance, out HitInfo aHitInfo) => InternalCalls.Physics_SphereCast(ref aOrigin, ref aDirection, aRadius, aMaxDistance, out aHitInfo);

        public static Entity[] OverlapSphere(Vector3 aOrigin, float aRadius) => InternalCalls.Physics_OverlapSphere(ref aOrigin, aRadius);

        public static void AddRadialImpulse(Vector3 aOrigin, float aRadius, float aStrength) => InternalCalls.Physics_AddRadialImpulse(ref aOrigin, aRadius, aStrength);
    }
}
