using System;

namespace Epoch
{
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public class ShowInEditorAttribute : Attribute
    {
        public string tooltip = "";
        public bool readOnly = false;

        public ShowInEditorAttribute(string aTooltip, bool aReadOnly = false)
        {
            tooltip = aTooltip;
            readOnly = aReadOnly;
        }

        public ShowInEditorAttribute(bool aReadOnly = false)
        {
            readOnly = aReadOnly;
        }
    }
}
