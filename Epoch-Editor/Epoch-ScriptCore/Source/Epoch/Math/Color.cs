using System;
using System.Runtime.InteropServices;

namespace Epoch
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Color : IEquatable<Color>
    {
        public float r;
        public float g;
        public float b;
        public float a;

        public static Color Zero = new Color(0, 0, 0, 0);
        public static Color One = new Color(1, 1, 1, 1);
        public static Color Black = new Color(0, 0, 0, 1);
        public static Color White = new Color(1, 1, 1, 1);
        public static Color Red = new Color(1, 0, 0, 1);
        public static Color Green = new Color(0, 1, 0, 1);
        public static Color Blue = new Color(0, 0, 1, 1);
        public static Color Cyan = new Color(0, 1, 1, 1);
        public static Color Magenta = new Color(1, 0, 1, 1);
        public static Color Yellow = new Color(1, 1, 0, 1);
        public static Color Orange = new Color(1, 0.65f, 0, 1);

        public Color(float aRGB, float aA)
        {
            r = aRGB;
            g = aRGB;
            b = aRGB;
            a = aA;
        }

        public Color(float aR, float aG, float aB, float aA)
        {
            r = (aR <= 1.0f) ? aR : aR / 255;
            g = (aG <= 1.0f) ? aG : aG / 255;
            b = (aB <= 1.0f) ? aB : aB / 255;
            a = aA;
        }

        public override bool Equals(object aObj) => aObj is Color aOther && Equals(aOther);
        public bool Equals(Color aOther) => r == aOther.r && g == aOther.g && b == aOther.b && a == aOther.a;
        public override int GetHashCode() => (r, g, b, a).GetHashCode();

        public static Color operator *(Color aCol0, float aScalar) => new Color(aCol0.r * aScalar, aCol0.g * aScalar, aCol0.b * aScalar, aCol0.a * aScalar);
        public static Color operator *(float aScalar, Color aCol1) => new Color(aScalar * aCol1.r, aScalar * aCol1.g, aScalar * aCol1.b, aScalar * aCol1.a);
        public static Color operator *(Color aCol0, Color aCol1) => new Color(aCol0.r * aCol1.r, aCol0.g * aCol1.g, aCol0.b * aCol1.b, aCol0.a * aCol1.a);
        public static Color operator +(Color aCol0, Color aCol1) => new Color(aCol0.r + aCol1.r, aCol0.g + aCol1.g, aCol0.b + aCol1.b, aCol0.a + aCol1.a);
        public static Color operator +(Color aCol0, float aScalar) => new Color(aCol0.r + aScalar, aCol0.g + aScalar, aCol0.b + aScalar, aCol0.a + aScalar);
        public static Color operator -(Color aCol0, Color aCol1) => new Color(aCol0.r - aCol1.r, aCol0.g - aCol1.g, aCol0.b - aCol1.b, aCol0.a - aCol1.a);
        public static Color operator -(Color aCol0, float aScalar) => new Color(aCol0.r - aScalar, aCol0.g - aScalar, aCol0.b - aScalar, aCol0.a - aScalar);
    }
}
