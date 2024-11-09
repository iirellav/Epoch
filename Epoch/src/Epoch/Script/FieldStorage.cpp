#include "epch.h"
#include "FieldStorage.h"

#include <mono/metadata/appdomain.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>

#include "ScriptUtils.h"
#include "ScriptEngine.h"


namespace Epoch
{
	bool FieldStorage::GetValueRuntime(Buffer& outBuffer) const
	{
		if (myRuntimeInstance == nullptr)
		{
			return false;
		}

		MonoObject* runtimeObject = GCManager::GetReferencedObject(myRuntimeInstance);
		if (runtimeObject == nullptr)
		{
			return false;
		}

		outBuffer = ScriptUtils::GetFieldValue(runtimeObject, myFieldInfo->name, myFieldInfo->type, myFieldInfo->isProperty);
		return true;
	}

	void FieldStorage::SetValueRuntime(const void* aData)
	{
		if (myRuntimeInstance == nullptr)
		{
			return;
		}

		MonoObject* runtimeObject = GCManager::GetReferencedObject(myRuntimeInstance);
		ScriptUtils::SetFieldValue(runtimeObject, myFieldInfo, aData);
	}


	void ArrayFieldStorage::Resize(uint32_t aNewLength)
	{
		if (myFieldInfo->type == FieldType::String)
		{
			return;
		}

		if (myRuntimeInstance == nullptr)
		{
			if (!myDataBuffer)
			{
				myDataBuffer.Allocate(aNewLength * myFieldInfo->size);
				myDataBuffer.ZeroInitialize();
			}
			else
			{
				Buffer newBuffer;
				newBuffer.Allocate(aNewLength * myFieldInfo->size);

				uint32_t copyLength = aNewLength < myLength ? aNewLength : myLength;
				memcpy(newBuffer.data, myDataBuffer.data, copyLength * myFieldInfo->size);

				if (aNewLength > myLength)
				{
					memset((byte*)newBuffer.data + (myLength * myFieldInfo->size), 0, (aNewLength - myLength) * myFieldInfo->size);
				}

				myDataBuffer.Release();
				myDataBuffer = newBuffer;
			}

			myLength = aNewLength;
		}
		else
		{
			MonoObject* runtimeObject = GCManager::GetReferencedObject(myRuntimeInstance);
			MonoClassField* field = mono_class_get_field_from_name(mono_object_get_class(runtimeObject), myFieldInfo->name.c_str());
			MonoClass* elementClass = mono_class_get_element_class(mono_type_get_class(mono_field_get_type(field)));
			MonoArray* arrayObject = (MonoArray*)mono_field_get_value_object(ScriptEngine::GetScriptDomain(), field, runtimeObject);
			MonoArray* newArray = mono_array_new(ScriptEngine::GetScriptDomain(), elementClass, aNewLength);

			if (arrayObject != nullptr)
			{
				uintptr_t length = mono_array_length(arrayObject);
				uintptr_t copyLength = aNewLength < length ? aNewLength : length;

				char* src = mono_array_addr_with_size(arrayObject, myFieldInfo->size, 0);
				char* dst = mono_array_addr_with_size(newArray, myFieldInfo->size, 0);
				memcpy(dst, src, copyLength * myFieldInfo->size);
			}

			mono_field_set_value(runtimeObject, field, newArray);
		}
	}

	void ArrayFieldStorage::RemoveAt(uint32_t aIndex)
	{
		if (myRuntimeInstance == nullptr)
		{
			Buffer newBuffer;
			newBuffer.Allocate((myLength - 1) * myFieldInfo->size);

			if (aIndex != 0)
			{
				memcpy(newBuffer.data, myDataBuffer.data, aIndex * myFieldInfo->size);
				memcpy((byte*)newBuffer.data + (aIndex * myFieldInfo->size), (byte*)myDataBuffer.data + ((aIndex + 1) * myFieldInfo->size), (myLength - aIndex - 1) * myFieldInfo->size);
			}
			else
			{
				memcpy(newBuffer.data, (byte*)myDataBuffer.data + myFieldInfo->size, (myLength - 1) * myFieldInfo->size);
			}

			myDataBuffer.Release();
			myDataBuffer = Buffer::Copy(newBuffer);
			myLength--;
		}
		else
		{
			MonoObject* runtimeObject = GCManager::GetReferencedObject(myRuntimeInstance);
			MonoClassField* field = mono_class_get_field_from_name(mono_object_get_class(runtimeObject), myFieldInfo->name.c_str());
			MonoArray* arrayObject = (MonoArray*)mono_field_get_value_object(ScriptEngine::GetScriptDomain(), field, runtimeObject);
			uint32_t length = (uint32_t)mono_array_length(arrayObject);
			EPOCH_ASSERT(aIndex < length, "Index out of range");

			if (aIndex == length - 1)
			{
				Resize(length - 1);
				return;
			}

			MonoClass* arrayClass = mono_object_get_class((MonoObject*)arrayObject);
			MonoClass* elementClass = mono_class_get_element_class(arrayClass);
			int32_t elementSize = mono_array_element_size(arrayClass);
			MonoArray* newArray = mono_array_new(ScriptEngine::GetScriptDomain(), elementClass, length - 1);

			if (aIndex != 0)
			{
				char* src = mono_array_addr_with_size(arrayObject, elementSize, 0);
				char* dst = mono_array_addr_with_size(newArray, elementSize, 0);
				memcpy(dst, src, aIndex * elementSize);

				src = mono_array_addr_with_size(arrayObject, elementSize, aIndex + 1);
				dst = mono_array_addr_with_size(newArray, elementSize, aIndex);
				memcpy(dst, src, (length - aIndex - 1) * elementSize);
			}
			else
			{
				char* src = mono_array_addr_with_size(arrayObject, elementSize, 1);
				char* dst = mono_array_addr_with_size(newArray, elementSize, 0);
				memcpy(dst, src, (length - 1) * elementSize);
			}

			mono_field_set_value(runtimeObject, field, newArray);
		}
	}

	bool ArrayFieldStorage::GetRuntimeArray(Buffer& outData) const
	{
		if (myRuntimeInstance == nullptr)
		{
			return false;
		}

		MonoObject* runtimeObject = GCManager::GetReferencedObject(myRuntimeInstance);
		MonoClassField* field = mono_class_get_field_from_name(mono_object_get_class(runtimeObject), myFieldInfo->name.c_str());
		MonoArray* arrayObject = (MonoArray*)mono_field_get_value_object(ScriptEngine::GetScriptDomain(), field, runtimeObject);
		
		if (arrayObject == nullptr)
		{
			return false;
		}

		MonoType* fieldType = mono_field_get_type(field);
		MonoClass* fieldTypeClass = mono_type_get_class(fieldType);
		MonoClass* fieldElementClass = mono_class_get_element_class(fieldTypeClass);
		MonoType* elementType = mono_class_get_type(fieldElementClass);

		uint32_t arrayLength = (uint32_t)mono_array_length(arrayObject);
		uint32_t bufferSize = arrayLength * myFieldInfo->size;
		outData.Allocate(bufferSize);
		outData.ZeroInitialize();

		if (mono_type_is_reference(elementType) || mono_type_is_byref(elementType))
		{
			for (uint32_t i = 0; i < arrayLength; i++)
			{
				MonoObject* obj = mono_array_get(arrayObject, MonoObject*, i);
				Buffer valueBuffer = ScriptUtils::MonoObjectToValue(obj, myFieldInfo->type);
				outData.Write(valueBuffer.data, valueBuffer.size, (uint64_t)(i * myFieldInfo->size));
				valueBuffer.Release();
			}
		}
		else
		{
			for (uint32_t i = 0; i < arrayLength; i++)
			{
				char* src = mono_array_addr_with_size(arrayObject, (int)myFieldInfo->size, i);
				outData.Write((const void*)src, (uint64_t)myFieldInfo->size, (uint64_t)(i * myFieldInfo->size));
			}
		}

		return true;
	}

	void ArrayFieldStorage::SetRuntimeArray(const Buffer& aData)
	{
		if (myRuntimeInstance == nullptr)
		{
			return;
		}

		if (!aData)
		{
			return;
		}

		MonoObject* runtimeObject = GCManager::GetReferencedObject(myRuntimeInstance);

		if (runtimeObject == nullptr)
		{
			return;
		}

		MonoClass* instanceClass = mono_object_get_class(runtimeObject);
		MonoClassField* field = mono_class_get_field_from_name(instanceClass, myFieldInfo->name.c_str());
		MonoType* fieldType = mono_field_get_type(field);
		MonoClass* fieldTypeClass = mono_type_get_class(fieldType);
		MonoClass* fieldElementClass = mono_class_get_element_class(fieldTypeClass);
		MonoType* elementType = mono_class_get_type(fieldElementClass);

		MonoArray* arr = mono_array_new(ScriptEngine::GetScriptDomain(), fieldElementClass, myLength);

		if (mono_type_is_reference(elementType) || mono_type_is_byref(elementType))
		{
			for (uint32_t i = 0; i < myLength; i++)
			{
				if (myFieldInfo->type == FieldType::String)
				{
					Buffer* valueData = static_cast<Buffer*>(aData.data) + i;
					MonoObject* boxed = ScriptUtils::ValueToMonoObject(valueData->data, ScriptUtils::GetFieldTypeFromMonoType(elementType));
					mono_array_setref(arr, i, boxed);
				}
				else
				{
					auto* element = static_cast<std::byte*>(aData.data) + i * myFieldInfo->size;
					MonoObject* boxed = ScriptUtils::ValueToMonoObject(element, ScriptUtils::GetFieldTypeFromMonoType(elementType));
					mono_array_setref(arr, i, boxed);
				}
			}
		}
		else
		{
			for (uint32_t i = 0; i < myLength; i++)
			{
				char* dst = mono_array_addr_with_size(arr, (int)myFieldInfo->size, i);
				memcpy(dst, static_cast<std::byte*>(aData.data) + i * myFieldInfo->size, myFieldInfo->size);
			}
		}

		mono_field_set_value(runtimeObject, field, arr);
	}

	void ArrayFieldStorage::GetValueRuntime(uint32_t aIndex, void* aData) const
	{
		if (myRuntimeInstance == nullptr)
		{
			return;
		}

		MonoObject* runtimeObject = GCManager::GetReferencedObject(myRuntimeInstance);
		MonoClassField* field = mono_class_get_field_from_name(mono_object_get_class(runtimeObject), myFieldInfo->name.c_str());
		MonoArray* arrayObject = (MonoArray*)mono_field_get_value_object(ScriptEngine::GetScriptDomain(), field, runtimeObject);

		if (!arrayObject)
		{
			return;
		}

		MonoType* fieldType = mono_field_get_type(field);
		MonoClass* fieldTypeClass = mono_type_get_class(fieldType);
		MonoClass* fieldElementClass = mono_class_get_element_class(fieldTypeClass);
		MonoType* elementType = mono_class_get_type(fieldElementClass);

		char* src = mono_array_addr_with_size(arrayObject, myFieldInfo->size, aIndex);
		memcpy(aData, src, myFieldInfo->size);

		if (mono_type_is_reference(elementType) || mono_type_is_byref(elementType))
		{
			MonoObject* obj = mono_array_get(arrayObject, MonoObject*, aIndex);
			Buffer valueBuffer = ScriptUtils::MonoObjectToValue(obj, myFieldInfo->type);
			memcpy(aData, valueBuffer.data, valueBuffer.size);
			valueBuffer.Release();
		}
		else
		{
			char* src = mono_array_addr_with_size(arrayObject, (int)myFieldInfo->size, aIndex);
			memcpy(aData, src, myFieldInfo->size);
		}
	}

	void ArrayFieldStorage::SetValueRuntime(uint32_t aIndex, const void* aData)
	{
		if (myRuntimeInstance == nullptr)
		{
			return;
		}

		MonoObject* runtimeObject = GCManager::GetReferencedObject(myRuntimeInstance);
		MonoClassField* field = mono_class_get_field_from_name(mono_object_get_class(runtimeObject), myFieldInfo->name.c_str());
		MonoArray* arrayObject = (MonoArray*)mono_field_get_value_object(ScriptEngine::GetScriptDomain(), field, runtimeObject);

		MonoType* fieldType = mono_field_get_type(field);
		MonoClass* fieldTypeClass = mono_type_get_class(fieldType);
		MonoClass* fieldElementClass = mono_class_get_element_class(fieldTypeClass);
		MonoType* elementType = mono_class_get_type(fieldElementClass);

		if (mono_type_is_reference(elementType) || mono_type_is_byref(elementType))
		{
			MonoObject* boxed = ScriptUtils::ValueToMonoObject(aData, ScriptUtils::GetFieldTypeFromMonoType(elementType));
			mono_array_setref(arrayObject, aIndex, boxed);
		}
		else
		{
			char* dst = mono_array_addr_with_size(arrayObject, (int)myFieldInfo->size, aIndex);
			memcpy(dst, aData, myFieldInfo->size);
		}
	}

	uint32_t ArrayFieldStorage::GetLengthRuntime() const
	{
		if (myRuntimeInstance == nullptr)
		{
			return 0;
		}

		MonoObject* runtimeObject = GCManager::GetReferencedObject(myRuntimeInstance);
		MonoClassField* field = mono_class_get_field_from_name(mono_object_get_class(runtimeObject), myFieldInfo->name.c_str());
		MonoArray* arrayObject = (MonoArray*)mono_field_get_value_object(ScriptEngine::GetScriptDomain(), field, runtimeObject);

		if (arrayObject == nullptr)
		{
			return 0;
		}

		return (uint32_t)mono_array_length(arrayObject);
	}
}
