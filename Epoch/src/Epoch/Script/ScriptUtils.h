#pragma once
#include <string>
#include <mono/utils/mono-error.h>
#include <mono/metadata/object.h>
#include "ScriptCache.h"
#include "ValueWrapper.h"
#include "Epoch/Scene/Scene.h"

extern "C"
{
	typedef struct _MonoType MonoType;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoString MonoString;
	typedef struct _MonoArray MonoArray;
	typedef struct _MonoException MonoException;
}

namespace Epoch
{
	class ScriptUtils
	{
	public:
		//static void Init();
		//static void Shutdown();
		
		static bool CheckMonoError(MonoError& aError);
		static void HandleException(MonoObject* aException);

		static Buffer GetFieldValue(MonoObject* aClassInstance, std::string_view aFieldName, FieldType aFieldType, bool aIsProperty);
		static MonoObject* GetFieldValueObject(MonoObject* aClassInstance, std::string_view aFieldName, bool aIsProperty);
		static void SetFieldValue(MonoObject* aClassInstance, const FieldInfo* aFieldInfo, const void* aData);
		
		static Buffer MonoObjectToValue(MonoObject* aObj, FieldType aFieldType);
		static MonoObject* ValueToMonoObject(const void* aData, FieldType aDataType);
		static FieldType GetFieldTypeFromMonoType(MonoType* aMonoType);

		static std::string ResolveMonoClassName(MonoClass* aMonoClass);
		
		static std::string MonoStringToUTF8(MonoString* aMonoString);
		static MonoString* UTF8StringToMono(const std::string& aStr);

		template<typename TValueType>
		static TValueType Unbox(MonoObject* aObj) { return *(TValueType*)mono_object_unbox(aObj); }
		template<typename TValueType>
		static TValueType UnboxAddress(MonoObject* aObj) { return (TValueType*)mono_object_unbox(aObj); }

		static MonoObject* BoxValue(MonoClass* aValueClass, const void* aValue);

	private:

	};

	class ManagedArrayUtils
	{
	public:
		//static Utils::ValueWrapper GetValue(MonoArray* aArr, uintptr_t aIndex);
	
		template<typename TValueType>
		static void SetValue(MonoArray* aArr, uintptr_t aIndex, TValueType aValue)
		{
			if constexpr (std::is_same<TValueType, MonoObject*>::value)
			{
				SetValueInternal(aArr, aIndex, aValue);
			}
			else
			{
				SetValueInternal(aArr, aIndex, &aValue);
			}
		}
	
		static uintptr_t Length(MonoArray* aArr);
		static void Resize(MonoArray** aArr, uintptr_t aNewLength);
		static MonoArray* Copy(MonoArray* aArr);
	
		template<typename TValueType>
		static MonoArray* FromVector(const std::vector<TValueType>& aVec)
		{
			MonoArray* arr = Create<TValueType>(aVec.size());
			for (size_t i = 0; i < aVec.size(); i++)
			{
				SetValue<TValueType>(arr, i, aVec[i]);
			}
			return arr;
		}
	
		//template<typename TValueType>
		//static std::vector<TValueType> ToVector(MonoArray* aArr)
		//{
		//	uintptr_t length = Length(aArr);
		//
		//	std::vector<TValueType> vec;
		//	vec.resize(length);
		//
		//	if constexpr (std::is_same_v<TValueType, Utils::ValueWrapper>)
		//	{
		//		for (uintptr_t i = 0; i < length; i++)
		//		{
		//			vec[i] = GetValue(aArr, i);
		//		}
		//	}
		//	else
		//	{
		//		for (uintptr_t i = 0; i < length; i++)
		//		{
		//			vec[i] = GetValue(aArr, i).Get<TValueType>();
		//		}
		//	}
		//
		//	return vec;
		//}
	
	public:
		static MonoArray* Create(const std::string& aArrayClass, uintptr_t aLength);
		static MonoArray* Create(ManagedClass* aArrayClass, uintptr_t aLength);
	
		template<typename T>
		static MonoArray* Create(uintptr_t aLength)
		{
			EPOCH_ASSERT(false, "No type specified!");
			return nullptr;
		}
	
		template<> static MonoArray* Create<bool>(uintptr_t aLength) { return Create("System.Boolean", aLength); }
		template<> static MonoArray* Create<int8_t>(uintptr_t aLength) { return Create("System.SByte", aLength); }
		template<> static MonoArray* Create<int16_t>(uintptr_t aLength) { return Create("System.Int16", aLength); }
		template<> static MonoArray* Create<int32_t>(uintptr_t aLength) { return Create("System.Int32", aLength); }
		template<> static MonoArray* Create<int64_t>(uintptr_t aLength) { return Create("System.Int64", aLength); }
		template<> static MonoArray* Create<uint8_t>(uintptr_t aLength) { return Create("System.Byte", aLength); }
		template<> static MonoArray* Create<uint16_t>(uintptr_t aLength) { return Create("System.UInt16", aLength); }
		template<> static MonoArray* Create<uint32_t>(uintptr_t aLength) { return Create("System.UInt32", aLength); }
		template<> static MonoArray* Create<uint64_t>(uintptr_t aLength) { return Create("System.UInt64", aLength); }
		template<> static MonoArray* Create<float>(uintptr_t aLength) { return Create("System.Single", aLength); }
		template<> static MonoArray* Create<double>(uintptr_t aLength) { return Create("System.Double", aLength); }
		template<> static MonoArray* Create<char>(uintptr_t aLength) { return Create("System.Char", aLength); }
		template<> static MonoArray* Create<std::string>(uintptr_t aLength) { return Create("System.String", aLength); }
		template<> static MonoArray* Create<Entity>(uintptr_t aLength) { return Create("Epoch.Entity", aLength); }
		template<> static MonoArray* Create<Prefab>(uintptr_t aLength) { return Create("Epoch.Prefab", aLength); }
		template<> static MonoArray* Create<CU::Vector2f>(uintptr_t aLength) { return Create("Epoch.Vector2", aLength); }
		template<> static MonoArray* Create<CU::Vector3f>(uintptr_t aLength) { return Create("Epoch.Vector3", aLength); }
		template<> static MonoArray* Create<CU::Color>(uintptr_t aLength) { return Create("Epoch.Color", aLength); }
	
	private:
		static void SetValueInternal(MonoArray* aArr, uintptr_t aIndex, void* aData);
		static void SetValueInternal(MonoArray* aArr, uintptr_t aIndex, MonoObject* aValue);
	};
}
