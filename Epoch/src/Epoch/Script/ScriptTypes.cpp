#include "epch.h"
#include "ScriptTypes.h"

#include <mono/metadata/class.h>
#include <mono/metadata/tokentype.h>
#include <mono/metadata/reflection.h>

#include "ScriptCache.h"
#include "ScriptUtils.h"

namespace Epoch::TypeUtils
{
	bool ContainsAttribute(void* aAttributeList, const std::string& aAttributeName)
	{
		const ManagedClass* attributeClass = ScriptCache::GetManagedClassByName(aAttributeName);

		if (attributeClass == nullptr)
		{
			return false;
		}

		if (aAttributeList == nullptr)
		{
			return false;
		}

		return mono_custom_attrs_has_attr((MonoCustomAttrInfo*)aAttributeList, attributeClass->monoClass);
	}
}
