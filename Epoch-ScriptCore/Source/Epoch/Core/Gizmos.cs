namespace Epoch
{
    public static class Gizmos
    {
        public static void DrawWireSphere(Vector3 aCenter, float aRadius, Color aColor) => InternalCalls.Gizmos_DrawWireSphere(ref aCenter, aRadius, ref aColor);
        public static void DrawWireCube(Vector3 aCenter, Vector3 aRotation, Vector3 aSize, Color aColor) => InternalCalls.Gizmos_DrawWireCube(ref aCenter, ref aRotation, ref aSize, ref aColor);
        public static void DrawLine(Vector3 aFrom, Vector3 aTo, Color aColor) => InternalCalls.Gizmos_DrawLine(ref aFrom, ref aTo, ref aColor);
    }
}
