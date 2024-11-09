#include "epch.h"
#include "CSharpInstanceInspector.h"
#include <mono/metadata/object.h>
#include "ScriptUtils.h"

namespace Epoch
{
	CSharpInstanceInspector::CSharpInstanceInspector(MonoObject* aInstance) : myInstance(aInstance)
	{
		MonoClass* instanceClass = mono_object_get_class(aInstance);
	}

	bool CSharpInstanceInspector::HasName(std::string_view aFullName) const
	{
		MonoClass* instanceClass = mono_object_get_class(myInstance);

		std::string_view className = mono_class_get_name(instanceClass);
		std::string_view classNamespace = mono_class_get_namespace(instanceClass);

		bool providedNamespace = aFullName.find_first_of(".") != std::string_view::npos;

		if (providedNamespace)
		{
			if (classNamespace.empty())
			{
				return false;
			}

			if (aFullName.find(classNamespace) == std::string_view::npos)
			{
				return false;
			}
		}

		return aFullName.find(className) != std::string_view::npos;
	}

	bool CSharpInstanceInspector::InheritsFrom(std::string_view aFullName) const
	{
		bool result = false;

		MonoClass* instanceClass = mono_object_get_class(myInstance);
		MonoClass* parentClass = mono_class_get_parent(instanceClass);

		bool providedNamespace = aFullName.find_first_of(".") != std::string_view::npos;

		while (parentClass != nullptr)
		{
			std::string_view parentNamespace = mono_class_get_namespace(parentClass);

			if (providedNamespace)
			{
				if (parentNamespace.empty())
				{
					// fullName can't possibly match this parents name
					result = false;
					parentClass = mono_class_get_parent(parentClass);
					continue;
				}

				if (aFullName.find(parentNamespace) == std::string_view::npos)
				{
					// Namespace doesn't match
					result = false;
					parentClass = mono_class_get_parent(parentClass);
					continue;
				}
			}

			std::string_view parentName = mono_class_get_name(parentClass);
			if (aFullName.find(parentName) != std::string_view::npos)
			{
				// Match
				result = true;
				break;
			}

			parentClass = mono_class_get_parent(parentClass);
		}

		return result;
	}

	const char* CSharpInstanceInspector::GetName() const
	{
		MonoClass* instanceClass = mono_object_get_class(myInstance);
		return mono_class_get_name(instanceClass);
	}

	Buffer CSharpInstanceInspector::GetFieldBuffer(std::string_view aFieldName) const
	{
		MonoClass* instanceClass = mono_object_get_class(myInstance);
		MonoClassField* field = mono_class_get_field_from_name(instanceClass, aFieldName.data());
		MonoProperty* prop = mono_class_get_property_from_name(instanceClass, aFieldName.data());

		if (field == nullptr && prop == nullptr)
		{
			// No field or property with this name was found
			return {};
		}

		MonoType* fieldMonoType = nullptr;

		if (field != nullptr)
		{
			fieldMonoType = mono_field_get_type(field);
		}
		else if (prop != nullptr)
		{
			MonoMethod* getterMethod = mono_property_get_get_method(prop);
			MonoMethod* setterMethod = mono_property_get_set_method(prop);

			if (getterMethod != nullptr)
			{
				fieldMonoType = mono_signature_get_return_type(mono_method_signature(getterMethod));
			}
			else if (setterMethod != nullptr)
			{
				void* paramIter = nullptr;
				fieldMonoType = mono_signature_get_params(mono_method_signature(setterMethod), &paramIter);
			}
			else
			{
				EPOCH_ASSERT(false, "No getter or setter?!");
			}
		}
		else
		{
			EPOCH_ASSERT(false, "What is this?");
		}
		
		FieldType fieldType = ScriptUtils::GetFieldTypeFromMonoType(fieldMonoType);
		return ScriptUtils::GetFieldValue(myInstance, aFieldName, fieldType, prop != nullptr);
	}
}