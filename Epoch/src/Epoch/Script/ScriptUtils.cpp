#include "epch.h"
#include "ScriptUtils.h"
#include <CommonUtilities/StringUtils.h>

#include <mono/metadata/appdomain.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/tokentype.h>

#include "ScriptEngine.h"

namespace Epoch
{
	struct MonoExceptionInfo
	{
		std::string typeName;
		std::string source;
		std::string message;
		std::string stackTrace;
	};

	static MonoExceptionInfo GetExceptionInfo(MonoObject* exception)
	{
		MonoClass* exceptionClass = mono_object_get_class(exception);
		MonoType* exceptionType = mono_class_get_type(exceptionClass);

		auto GetExceptionString = [exception, exceptionClass](const char* stringName) -> std::string
		{
			MonoProperty* property = mono_class_get_property_from_name(exceptionClass, stringName);

			if (property == nullptr)
				return "";

			MonoMethod* getterMethod = mono_property_get_get_method(property);

			if (getterMethod == nullptr)
				return "";

			MonoString* string = (MonoString*)mono_runtime_invoke(getterMethod, exception, NULL, NULL);
			return ScriptUtils::MonoStringToUTF8(string);
		};

		return { mono_type_get_name(exceptionType), GetExceptionString("Source"), GetExceptionString("Message"), GetExceptionString("StackTrace") };
	}

	bool ScriptUtils::CheckMonoError(MonoError& aError)
	{
		bool hasError = !mono_error_ok(&aError);

		if (hasError)
		{
			unsigned short errorCode = mono_error_get_error_code(&aError);
			const char* errorMessage = mono_error_get_message(&aError);

			LOG_ERROR_TAG("ScriptEngine", "Mono Error!");
			LOG_ERROR_TAG("ScriptEngine", "\tError Code: {}", errorCode);
			LOG_ERROR_TAG("ScriptEngine", "\tError Message: {}", errorMessage);
			mono_error_cleanup(&aError);
			EPOCH_ASSERT(false, "Fatal Error!");
		}

		return hasError;
	}

	void ScriptUtils::HandleException(MonoObject* aException)
	{
		if (aException == nullptr) return;

		MonoExceptionInfo exceptionInfo = GetExceptionInfo(aException);
		CONSOLE_LOG_ERROR("{}: {}. Source: {}, Stack Trace: {}", exceptionInfo.typeName, exceptionInfo.message, exceptionInfo.source, exceptionInfo.stackTrace);
	}

	Buffer ScriptUtils::GetFieldValue(MonoObject* aClassInstance, std::string_view aFieldName, FieldType aFieldType, bool aIsProperty)
	{
		return MonoObjectToValue(GetFieldValueObject(aClassInstance, aFieldName, aIsProperty), aFieldType);
	}

	MonoObject* ScriptUtils::GetFieldValueObject(MonoObject* aClassInstance, std::string_view aFieldName, bool aIsProperty)
	{
		MonoClass* objectClass = mono_object_get_class(aClassInstance);

		MonoObject* valueObject = nullptr;

		if (aIsProperty)
		{
			MonoProperty* classProperty = mono_class_get_property_from_name(objectClass, aFieldName.data());
			valueObject = mono_property_get_value(classProperty, aClassInstance, nullptr, nullptr);
		}
		else
		{
			MonoClassField* classField = mono_class_get_field_from_name(objectClass, aFieldName.data());
			valueObject = mono_field_get_value_object(mono_domain_get(), classField, aClassInstance);
		}

		return valueObject;
	}

	void ScriptUtils::SetFieldValue(MonoObject* aClassInstance, const FieldInfo* aFieldInfo, const void* aData)
	{
		if (aClassInstance == nullptr || aFieldInfo == nullptr || aData == nullptr)
		{
			return;
		}

		if (!aFieldInfo->IsWritable())
		{
			return;
		}

		MonoClass* objectClass = mono_object_get_class(aClassInstance);

		if (aFieldInfo->isProperty)
		{
			MonoProperty* classProperty = mono_class_get_property_from_name(objectClass, aFieldInfo->name.c_str());
			void* propertyData = nullptr;

			if (aFieldInfo->IsArray() || FieldUtils::IsPrimitiveType(aFieldInfo->type))
			{
				propertyData = const_cast<void*>(aData);
			}
			else
			{
				propertyData = ValueToMonoObject(aData, aFieldInfo->type);
			}

			mono_property_set_value(classProperty, aClassInstance, &propertyData, nullptr);
		}
		else
		{
			MonoClassField* classField = mono_class_get_field_from_name(objectClass, aFieldInfo->name.c_str());
			void* fieldData = nullptr;

			if (aFieldInfo->IsArray() || FieldUtils::IsPrimitiveType(aFieldInfo->type))
			{
				fieldData = (void*)aData;
			}
			else
			{
				fieldData = ValueToMonoObject(aData, aFieldInfo->type);
			}

			mono_field_set_value(aClassInstance, classField, fieldData);
		}
	}

	Buffer ScriptUtils::MonoObjectToValue(MonoObject* aObj, FieldType aFieldType)
	{
		if (aObj == nullptr)
		{
			return Buffer();
		}

		Buffer result;
		result.Allocate(FieldUtils::GetFieldTypeSize(aFieldType));
		result.ZeroInitialize();

		switch (aFieldType)
		{
			case FieldType::Bool:
			{
				bool value = (bool)Unbox<MonoBoolean>(aObj);
				result.Write(&value, sizeof(bool));
				break;
			}
			case FieldType::Int8:
			{
				int8_t value = Unbox<int8_t>(aObj);
				result.Write(&value, sizeof(int8_t));
				break;
			}
			case FieldType::Int16:
			{
				int16_t value = Unbox<int16_t>(aObj);
				result.Write(&value, sizeof(int16_t));
				break;
			}
			case FieldType::Int32:
			{
				int32_t value = Unbox<int32_t>(aObj);
				result.Write(&value, sizeof(int32_t));
				break;
			}
			case FieldType::Int64:
			{
				int64_t value = Unbox<int64_t>(aObj);
				result.Write(&value, sizeof(int64_t));
				break;
			}
			case FieldType::UInt8:
			{
				uint8_t value = Unbox<uint8_t>(aObj);
				result.Write(&value, sizeof(uint8_t));
				break;
			}
			case FieldType::UInt16:
			{
				uint16_t value = Unbox<uint16_t>(aObj);
				result.Write(&value, sizeof(uint16_t));
				break;
			}
			case FieldType::LayerMask:
			case FieldType::UInt32:
			{
				uint32_t value = Unbox<uint32_t>(aObj);
				result.Write(&value, sizeof(uint32_t));
				break;
			}
			case FieldType::UInt64:
			{
				uint64_t value = Unbox<uint64_t>(aObj);
				result.Write(&value, sizeof(uint64_t));
				break;
			}
			case FieldType::Float:
			{
				float value = Unbox<float>(aObj);
				result.Write(&value, sizeof(float));
				break;
			}
			case FieldType::Double:
			{
				double value = Unbox<double>(aObj);
				result.Write(&value, sizeof(double));
				break;
			}
			case FieldType::String:
			{
				std::string str = MonoStringToUTF8((MonoString*)aObj);
				result.Allocate(str.size() + 1);
				result.ZeroInitialize();
				result.Write(str.data(), str.size());
				break;
			}
			case FieldType::AssetHandle:
			{
				AssetHandle value = Unbox<AssetHandle>(aObj);
				result.Write(&value, sizeof(AssetHandle));
				break;
			}
			case FieldType::Vector2:
			{
				CU::Vector2f value = Unbox<CU::Vector2f>(aObj);
				result.Write(&value.x, sizeof(CU::Vector2f));
				break;
			}
			case FieldType::Vector3:
			{
				CU::Vector3f value = Unbox<CU::Vector3f>(aObj);
				result.Write(&value.x, sizeof(CU::Vector3f));
				break;
			}
			case FieldType::Color:
			{
				CU::Color value = Unbox<CU::Color>(aObj);
				result.Write(&value.r, sizeof(CU::Color));
				break;
			}
			case FieldType::Entity:
			{
				Buffer idBuffer = GetFieldValue(aObj, "id", FieldType::UInt64, false);
				result.Write(idBuffer.data, sizeof(UUID));
				idBuffer.Release();
				break;
			}
			case FieldType::Scene:
			case FieldType::Prefab:
			case FieldType::Material:
			case FieldType::Mesh:
			case FieldType::Texture2D:
			{
				Buffer handleBuffer = GetFieldValue(aObj, "myHandle", FieldType::AssetHandle, false);
				result.Write(handleBuffer.data, sizeof(AssetHandle));
				handleBuffer.Release();
				break;
			}
		}

		return result;
	}

	MonoObject* ScriptUtils::ValueToMonoObject(const void* aData, FieldType aDataType)
	{
		if (FieldUtils::IsPrimitiveType(aDataType))
		{
			return BoxValue(ScriptCache::GetFieldTypeClass(aDataType), aData);
		}
		else
		{
			switch (aDataType)
			{
				case FieldType::String: return (MonoObject*)UTF8StringToMono(std::string((const char*)aData));
				case FieldType::Scene: return ScriptEngine::CreateManagedObject("Epoch.Scene", *(AssetHandle*)aData);
				case FieldType::Prefab: return ScriptEngine::CreateManagedObject("Epoch.Prefab", *(AssetHandle*)aData);
				case FieldType::Material: return ScriptEngine::CreateManagedObject("Epoch.Material", *(AssetHandle*)aData);
				case FieldType::Mesh : return ScriptEngine::CreateManagedObject("Epoch.Mesh", *(AssetHandle*)aData);
				case FieldType::Texture2D: return ScriptEngine::CreateManagedObject("Epoch.Texture2D", *(AssetHandle*)aData);
				case FieldType::Entity: return ScriptEngine::CreateManagedObject("Epoch.Entity", *(UUID*)aData);
			}
		}

		EPOCH_ASSERT(false, "Unsupported value type!");
		return nullptr;
	}

	FieldType ScriptUtils::GetFieldTypeFromMonoType(MonoType* aMonoType)
	{
		int32_t typeEncoding = mono_type_get_type(aMonoType);
		MonoClass* typeClass = mono_type_get_class(aMonoType);

		switch (typeEncoding)
		{
			case MONO_TYPE_VOID:		return FieldType::Void;
			case MONO_TYPE_BOOLEAN:		return FieldType::Bool;
			case MONO_TYPE_CHAR:		return FieldType::UInt16;
			case MONO_TYPE_I1:			return FieldType::Int8;
			case MONO_TYPE_I2:			return FieldType::Int16;
			case MONO_TYPE_I4:			return FieldType::Int32;
			case MONO_TYPE_I8:			return FieldType::Int64;
			case MONO_TYPE_U1:			return FieldType::UInt8;
			case MONO_TYPE_U2:			return FieldType::UInt16;
			case MONO_TYPE_U4:			return FieldType::UInt32;
			case MONO_TYPE_U8:			return FieldType::UInt64;
			case MONO_TYPE_R4:			return FieldType::Float;
			case MONO_TYPE_R8:			return FieldType::Double;
			case MONO_TYPE_STRING:		return FieldType::String;
			case MONO_TYPE_VALUETYPE:
			{
				if (mono_class_is_enum(typeClass))
				{
					return GetFieldTypeFromMonoType(mono_type_get_underlying_type(aMonoType));
				}

				if (EPOCH_CORE_CLASS(AssetHandle) && typeClass == EPOCH_CORE_CLASS(AssetHandle)->monoClass)
				{
					return FieldType::AssetHandle;
				}

				if (EPOCH_CORE_CLASS(LayerMask) && typeClass == EPOCH_CORE_CLASS(LayerMask)->monoClass)
				{
					return FieldType::LayerMask;
				}

				if (EPOCH_CORE_CLASS(Vector2) && typeClass == EPOCH_CORE_CLASS(Vector2)->monoClass)
				{
					return FieldType::Vector2;
				}

				if (EPOCH_CORE_CLASS(Vector3) && typeClass == EPOCH_CORE_CLASS(Vector3)->monoClass)
				{
					return FieldType::Vector3;
				}

				if (EPOCH_CORE_CLASS(Color) && typeClass == EPOCH_CORE_CLASS(Color)->monoClass)
				{
					return FieldType::Color;
				}

				break;
			}
			case MONO_TYPE_CLASS:
			{
				auto entityClass = EPOCH_CORE_CLASS(Entity);
				
				if (entityClass && mono_class_is_assignable_from(typeClass, entityClass->monoClass)) return FieldType::Entity;

				if (EPOCH_CORE_CLASS(Scene) && typeClass == EPOCH_CORE_CLASS(Scene)->monoClass) return FieldType::Scene;
				if (EPOCH_CORE_CLASS(Prefab) && typeClass == EPOCH_CORE_CLASS(Prefab)->monoClass) return FieldType::Prefab;
				if (EPOCH_CORE_CLASS(Material) && typeClass == EPOCH_CORE_CLASS(Material)->monoClass) return FieldType::Material;
				if (EPOCH_CORE_CLASS(Mesh) && typeClass == EPOCH_CORE_CLASS(Mesh)->monoClass) return FieldType::Mesh;
				if (EPOCH_CORE_CLASS(Texture2D) && typeClass == EPOCH_CORE_CLASS(Texture2D)->monoClass) return FieldType::Texture2D;

				break;
			}
			case MONO_TYPE_SZARRAY:
			case MONO_TYPE_ARRAY:
			{
				MonoClass* elementClass = mono_class_get_element_class(typeClass);
				if (elementClass == nullptr)
				{
					break;
				}

				ManagedClass* managedElementClass = ScriptCache::GetManagedClass(elementClass);
				if (managedElementClass == nullptr)
				{
					break;
				}

				return GetFieldTypeFromMonoType(mono_class_get_type(elementClass));
			}
		}

		return FieldType::Void;
	}

	std::string ScriptUtils::ResolveMonoClassName(MonoClass* aMonoClass)
	{
		const char* classNamePtr = mono_class_get_name(aMonoClass);
		std::string className = classNamePtr != nullptr ? classNamePtr : "";

		if (className.empty())
		{
			return "Unknown Class";
		}

		MonoClass* nestingClass = mono_class_get_nesting_type(aMonoClass);
		if (nestingClass != nullptr)
		{
			className = ResolveMonoClassName(nestingClass) + "/" + className;
		}
		else
		{
			const char* classNamespacePtr = mono_class_get_namespace(aMonoClass);
			if (classNamespacePtr)
			{
				className = std::string(classNamespacePtr) + "." + className;
			}
		}

		MonoType* classType = mono_class_get_type(aMonoClass);
		if (mono_type_get_type(classType) == MONO_TYPE_SZARRAY || mono_type_get_type(classType) == MONO_TYPE_ARRAY)
		{
			CU::Erase(className, "[]");
		}

		return className;
	}

	std::string ScriptUtils::MonoStringToUTF8(MonoString* aMonoString)
	{
		if (aMonoString == nullptr || mono_string_length(aMonoString) == 0)
		{
			return "";
		}

		MonoError error;
		char* utf8 = mono_string_to_utf8_checked(aMonoString, &error);
		if (ScriptUtils::CheckMonoError(error))
		{
			return "";
		}
		std::string result(utf8);
		mono_free(utf8);
		return result;
	}

	MonoString* ScriptUtils::UTF8StringToMono(const std::string& aStr)
	{
		return mono_string_new(ScriptEngine::GetScriptDomain(), aStr.c_str());
	}

	MonoObject* ScriptUtils::BoxValue(MonoClass* aValueClass, const void* aValue)
	{
		return mono_value_box(ScriptEngine::GetScriptDomain(), aValueClass, const_cast<void*>(aValue));
	}


	uintptr_t ManagedArrayUtils::Length(MonoArray* aArr)
	{
		EPOCH_ASSERT(aArr, "aArr was null!");
		return mono_array_length(aArr);
	}
	
	void ManagedArrayUtils::Resize(MonoArray** aArr, uintptr_t aNewLength)
	{
		if (aArr == nullptr || *aArr == nullptr)
		{
			return;
		}
	
		MonoClass* arrayClass = mono_object_get_class((MonoObject*)*aArr);
		MonoClass* elementClass = mono_class_get_element_class(arrayClass);
	
		MonoArray* newArray = mono_array_new(ScriptEngine::GetScriptDomain(), elementClass, aNewLength);
	
		uintptr_t length = mono_array_length(*aArr);
		uintptr_t copyLength = aNewLength < length ? aNewLength : length;
	
		char* src = mono_array_addr_with_size(*aArr, mono_array_element_size(arrayClass), 0);
		char* dst = mono_array_addr_with_size(newArray, mono_array_element_size(arrayClass), 0);
		memcpy(dst, src, copyLength * mono_array_element_size(arrayClass));
	
		*aArr = newArray;
	}
	
	MonoArray* ManagedArrayUtils::Copy(MonoArray* aArr)
	{
		EPOCH_ASSERT(aArr, "aArr was null!");
	
		// TODO: Maybe attempt a deep copy? mono_array_clone only creates a shallow copy, for now that's fine though.
		return mono_array_clone(aArr);
	}
	
	MonoArray* ManagedArrayUtils::Create(const std::string& aArrayClass, uintptr_t aLength)
	{
		EPOCH_ASSERT(!aArrayClass.empty(), "Cannot create managed array of no type");
	
		ManagedClass* klass = ScriptCache::GetManagedClassByName(aArrayClass);
		EPOCH_ASSERT(klass, "Unable to find array class");
	
		return mono_array_new(ScriptEngine::GetScriptDomain(), klass->monoClass, aLength);
	}
	
	MonoArray* ManagedArrayUtils::Create(ManagedClass* aArrayClass, uintptr_t aLength)
	{
		EPOCH_ASSERT(aArrayClass, "Cannot create managed array of no type");
		return mono_array_new(ScriptEngine::GetScriptDomain(), aArrayClass->monoClass, aLength);
	}
	
	void ManagedArrayUtils::SetValueInternal(MonoArray* aArr, uintptr_t aIndex, void* aData)
	{
		EPOCH_ASSERT(aArr, "aArr was null!");
	
		uintptr_t length = mono_array_length(aArr);
		
		if (aIndex >= length)
		{
			LOG_WARNING_TAG("ScriptEngine", "Index out of bounds in C# array!");
			return;
		}
	
		MonoClass* arrayClass = mono_object_get_class((MonoObject*)aArr);
		MonoClass* elementClass = mono_class_get_element_class(arrayClass);
		int32_t elementSize = mono_array_element_size(arrayClass);
		MonoType* elementType = mono_class_get_type(elementClass);
	
		if (mono_type_is_reference(elementType) || mono_type_is_byref(elementType))
		{
			MonoObject* boxed = ScriptUtils::ValueToMonoObject(aData, ScriptUtils::GetFieldTypeFromMonoType(elementType));
			mono_array_setref(aArr, aIndex, boxed);
		}
		else
		{
			char* dst = mono_array_addr_with_size(aArr, elementSize, aIndex);
			memcpy(dst, aData, elementSize);
		}
	}
	
	void ManagedArrayUtils::SetValueInternal(MonoArray* aArr, uintptr_t aIndex, MonoObject* aValue)
	{
		EPOCH_ASSERT(aArr, "aArr was null!");
	
		uintptr_t length = mono_array_length(aArr);
		EPOCH_ASSERT(aIndex < length, "index out of range");
	
		MonoClass* arrayClass = mono_object_get_class((MonoObject*)aArr);
		MonoClass* elementClass = mono_class_get_element_class(arrayClass);
		int32_t elementSize = mono_array_element_size(arrayClass);
		MonoType* elementType = mono_class_get_type(elementClass);
	
		if (mono_type_is_reference(elementType) || mono_type_is_byref(elementType))
		{
			mono_array_setref(aArr, aIndex, aValue);
		}
		else
		{
			mono_array_set(aArr, MonoObject*, aIndex, aValue);
		}
	}
}