using System;

namespace Epoch
{
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public class HideFromEditorAttribute : Attribute
    {
    }
}
