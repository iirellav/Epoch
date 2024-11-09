namespace Epoch
{
    internal static class FormatUtils
    {
        internal static string Format(string aFormat, object[] aParameters) => string.Format(aFormat, aParameters);
        internal static string Format(object aValue) => aValue != null ? aValue.ToString() : "null";
    }
}
