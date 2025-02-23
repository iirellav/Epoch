using System;

namespace Epoch
{
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public class SpacingAttribute : Attribute
    {
        public uint spacing = 1;

        public SpacingAttribute(uint aSpacing = 1)
        {
            spacing = aSpacing;
        }
    }
}
