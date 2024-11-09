using System;

namespace Epoch
{
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public class HeaderAttribute : Attribute
    {
        public string header = "";

        public HeaderAttribute(string aHeader)
        {
            header = aHeader;
        }
    }
}
