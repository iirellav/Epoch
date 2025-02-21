using System.Runtime.InteropServices;

namespace Epoch
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Transform
    {
        public Vector3 position;
        public Vector3 rotation;
        public Vector3 scale;

        public Transform(Vector3 aPosition)
        {
            position = aPosition;
            rotation = Vector3.Zero;
            scale = Vector3.One;
        }

        public Transform(Vector3 aPosition, Vector3 aRotation)
        {
            position = aPosition;
            rotation = aRotation;
            scale = Vector3.One;
        }

        public Transform(Vector3 aPosition, Vector3 aRotation, Vector3 aScale)
        {
            position = aPosition;
            rotation = aRotation;
            scale = aScale;
        }

        public Vector3 Right { get => new Quaternion(rotation).GetRight(); }
        public Vector3 Up { get => new Quaternion(rotation).GetUp(); }
        public Vector3 Forward { get => new Quaternion(rotation).GetForward(); }
    }
}
