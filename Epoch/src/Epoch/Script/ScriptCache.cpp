#include "epch.h"
#include "ScriptCache.h"

#include <mono/metadata/assembly.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/tokentype.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/appdomain.h>

#include "Epoch/Core/Application.h"
#include "Epoch/Core/Hash.h"
#include "ScriptEngine.h"
#include "ScriptUtils.h"
#include "ScriptAsset.h"
#include "CSharpInstanceInspector.h"
#include "Epoch/Assets/AssetManager.h"

namespace Epoch
{
	struct Cache
	{
		std::unordered_map<uint32_t, ManagedClass> classes;
		std::unordered_map<uint32_t, FieldInfo> fields;
		std::unordered_map<uint32_t, std::vector<ManagedMethod>> methods;
	};

	static Cache* staticCache = nullptr;

	void ScriptCache::Init()
	{
		EPOCH_ASSERT(!staticCache, "Trying to initialize ScriptCache multiple times!");
		staticCache = new Cache();

		CacheCoreClasses();
	}

	void ScriptCache::Shutdown()
	{
		ClearCache();

		delete staticCache;
		staticCache = nullptr;
	}

	void ScriptCache::CacheCoreClasses()
	{
		if (staticCache == nullptr) return;

#define CACHE_CORELIB_CLASS(name) CacheClass("System." name, mono_class_from_name(mono_get_corlib(), "System", name))

		CACHE_CORELIB_CLASS("Object");
		CACHE_CORELIB_CLASS("ValueType");
		CACHE_CORELIB_CLASS("Boolean");
		CACHE_CORELIB_CLASS("SByte");
		CACHE_CORELIB_CLASS("Int16");
		CACHE_CORELIB_CLASS("Int32");
		CACHE_CORELIB_CLASS("Int64");
		CACHE_CORELIB_CLASS("Byte");
		CACHE_CORELIB_CLASS("UInt16");
		CACHE_CORELIB_CLASS("UInt32");
		CACHE_CORELIB_CLASS("UInt64");
		CACHE_CORELIB_CLASS("Single");
		CACHE_CORELIB_CLASS("Double");
		CACHE_CORELIB_CLASS("Char");
		CACHE_CORELIB_CLASS("String");
		CacheClass("System.Diagnostics.StackTrace", mono_class_from_name(mono_get_corlib(), "System.Diagnostics", "StackTrace"));

#define CACHE_EPOCH_CORE_CLASS(name) CacheClass("Epoch." name, mono_class_from_name(ScriptEngine::GetCoreAssemblyInfo()->assemblyImage, "Epoch", name))
		
		CACHE_EPOCH_CORE_CLASS("ShowInEditorAttribute");
		CACHE_EPOCH_CORE_CLASS("HideFromEditorAttribute");
		CACHE_EPOCH_CORE_CLASS("SpacingAttribute");
		CACHE_EPOCH_CORE_CLASS("HeaderAttribute");

		CACHE_EPOCH_CORE_CLASS("AssetHandle");
		CACHE_EPOCH_CORE_CLASS("LayerMask");
		CACHE_EPOCH_CORE_CLASS("Vector2");
		CACHE_EPOCH_CORE_CLASS("Vector3");
		CACHE_EPOCH_CORE_CLASS("Color");
		CACHE_EPOCH_CORE_CLASS("Scene");
		CACHE_EPOCH_CORE_CLASS("Entity");
		CACHE_EPOCH_CORE_CLASS("Prefab");
		CACHE_EPOCH_CORE_CLASS("Mesh");
		CACHE_EPOCH_CORE_CLASS("Texture2D");
		CACHE_EPOCH_CORE_CLASS("Material");
	}

	void ScriptCache::ClearCache()
	{
		if (staticCache == nullptr)
		{
			return;
		}

		// Remove all ScriptAssets
		if (!Application::IsRuntime())
		{
			for (const auto& [classID, classInfo] : staticCache->classes)
			{
				AssetHandle scriptAssetHandle = AssetHandle(Hash::GenerateFNVHash(classInfo.fullName));
				Project::GetEditorAssetManager()->RemoveAsset(scriptAssetHandle);
			}
		}

		staticCache->fields.clear();
		staticCache->methods.clear();
		staticCache->classes.clear();
	}

	static void BuildClassMetadata(std::shared_ptr<AssemblyInfo>& aAssemblyInfo, MonoClass* aMonoClass)
	{
		EPOCH_PROFILE_FUNC();
		
		EPOCH_ASSERT(aMonoClass, "MonoClass was null!");

		const std::string fullName = ScriptUtils::ResolveMonoClassName(aMonoClass);

		// C# adds a .<PrivateImplementationDetails> class for some reason?
		if (fullName.find("<PrivateImpl") != std::string::npos)
		{
			return;
		}

		uint32_t classID = (uint32_t)Hash::GenerateFNVHash(fullName);
		ManagedClass& managedClass = staticCache->classes[classID];
		managedClass.fullName = fullName;
		managedClass.id = classID;
		managedClass.monoClass = aMonoClass;
		uint32_t classFlags = mono_class_get_flags(aMonoClass);
		managedClass.isAbstract = classFlags & MONO_TYPE_ATTR_ABSTRACT;
		managedClass.isStruct = mono_class_is_valuetype(aMonoClass);
		managedClass.isEnum = mono_class_is_enum(aMonoClass);

		MonoClass* parentClass = mono_class_get_parent(aMonoClass);
		if (parentClass != nullptr && parentClass != EPOCH_CACHED_CLASS_RAW("System.Object"))
		{
			std::string parentName = ScriptUtils::ResolveMonoClassName(parentClass);
			managedClass.parentID = (uint32_t)Hash::GenerateFNVHash(parentName);
		}

		aAssemblyInfo->classes.push_back(managedClass.id);
	}

	void ScriptCache::GenerateCacheForAssembly(std::shared_ptr<AssemblyInfo> aAssemblyInfo)
	{
		EPOCH_PROFILE_FUNC();
		
		const MonoTableInfo* tableInfo = mono_image_get_table_info(aAssemblyInfo->assemblyImage, MONO_TABLE_TYPEDEF);
		int32_t tableRowCount = mono_table_info_get_rows(tableInfo);
		for (int32_t i = 1; i < tableRowCount; i++)
		{
			MonoClass* monoClass = mono_class_get(aAssemblyInfo->assemblyImage, (i + 1) | MONO_TOKEN_TYPE_DEF);
			BuildClassMetadata(aAssemblyInfo, monoClass);
		}

		// Process fields and properties after all classes have been parsed.
		for (auto classID : aAssemblyInfo->classes)
		{
			ManagedClass& managedClass = staticCache->classes.at(classID);

			CacheClassMethods(aAssemblyInfo, managedClass);

			MonoObject* tempInstance = ScriptEngine::CreateManagedObject_Internal(&managedClass);
			if (tempInstance == nullptr)
			{
				continue;
			}

			CacheClassFields(aAssemblyInfo, managedClass);
			CacheClassProperties(aAssemblyInfo, managedClass);

			if (mono_class_is_subclass_of(managedClass.monoClass, EPOCH_CACHED_CLASS_RAW("Epoch.Entity"), false))
			{
				AssetHandle handle = AssetManager::CreateMemoryOnlyAssetWithName<ScriptAsset>(managedClass.fullName, classID);
			}
		}

		for (auto classID : aAssemblyInfo->classes)
		{
			ManagedClass& managedClass = staticCache->classes.at(classID);
		
			if (!mono_class_is_subclass_of(managedClass.monoClass, EPOCH_CACHED_CLASS_RAW("Epoch.Entity"), false))
			{
				continue;
			}
		
			MonoObject* tempInstance = ScriptEngine::CreateManagedObject_Internal(&managedClass);
		
			for (auto fieldID : managedClass.fields)
			{
				FieldInfo& fieldInfo = staticCache->fields[fieldID];
				if (!fieldInfo.IsArray())
				{
					fieldInfo.defaultValueBuffer = ScriptUtils::GetFieldValue(tempInstance, fieldInfo.name, fieldInfo.type, fieldInfo.isProperty);
				}
				else
				{
					MonoArray* arr = (MonoArray*)ScriptUtils::GetFieldValueObject(tempInstance, fieldInfo.name, fieldInfo.isProperty);
		
					if (arr != nullptr)
					{
						fieldInfo.defaultValueBuffer.Allocate(mono_array_length(arr) * fieldInfo.size);
						fieldInfo.defaultValueBuffer.ZeroInitialize();
					}
				}
			}
		}
	}

	ManagedClass* ScriptCache::GetManagedClassByName(const std::string& aClassName)
	{
		if (staticCache == nullptr)
		{
			return nullptr;
		}

		const uint32_t classID = (uint32_t)Hash::GenerateFNVHash(aClassName);
		if (staticCache->classes.find(classID) == staticCache->classes.end())
		{
			return nullptr;
		}

		return &staticCache->classes[classID];
	}

	ManagedClass* ScriptCache::GetManagedClassByID(uint32_t aClassID)
	{
		if (staticCache == nullptr)
		{
			return nullptr;
		}

		if (staticCache->classes.find(aClassID) == staticCache->classes.end())
		{
			return nullptr;
		}

		return &staticCache->classes[aClassID];
	}

	ManagedClass* ScriptCache::GetManagedClass(MonoClass* aMonoClass)
	{
		if (staticCache == nullptr)
		{
			return nullptr;
		}

		if (aMonoClass == nullptr)
		{
			return nullptr;
		}

		return GetManagedClassByName(ScriptUtils::ResolveMonoClassName(aMonoClass));
	}

	ManagedClass* ScriptCache::GetMonoObjectClass(MonoObject* aMonoObject)
	{
		if (staticCache == nullptr)
		{
			return nullptr;
		}

		MonoClass* objectClass = mono_object_get_class(aMonoObject);
		if (objectClass == nullptr)
		{
			return nullptr;
		}

		return GetManagedClassByName(ScriptUtils::ResolveMonoClassName(objectClass));
	}

	FieldInfo* ScriptCache::GetFieldByID(uint32_t aFieldID)
	{
		if (staticCache == nullptr)
		{
			return nullptr;
		}

		if (staticCache->fields.find(aFieldID) == staticCache->fields.end())
		{
			return nullptr;
		}

		return &staticCache->fields.at(aFieldID);
	}

	FieldInfo* ScriptCache::GetFieldByName(const ManagedClass* aManagedClass, const std::string& aFieldName)
	{
		const uint32_t fieldID = (uint32_t)Hash::GenerateFNVHash(aManagedClass->fullName + ":" + aFieldName);
		if (staticCache->fields.find(fieldID) == staticCache->fields.end())
		{
			return nullptr;
		}

		return &staticCache->fields[fieldID];
	}

	MonoClass* ScriptCache::GetFieldTypeClass(FieldType aFieldType)
	{
		switch (aFieldType)
		{
			case FieldType::Bool:			return EPOCH_CACHED_CLASS("System.Bool")->monoClass;
			case FieldType::Int8:			return EPOCH_CACHED_CLASS("System.SByte")->monoClass;
			case FieldType::Int16:			return EPOCH_CACHED_CLASS("System.Int16")->monoClass;
			case FieldType::Int32:			return EPOCH_CACHED_CLASS("System.Int32")->monoClass;
			case FieldType::Int64:			return EPOCH_CACHED_CLASS("System.Int64")->monoClass;
			case FieldType::UInt8:			return EPOCH_CACHED_CLASS("System.Byte")->monoClass;
			case FieldType::UInt16:			return EPOCH_CACHED_CLASS("System.UInt16")->monoClass;
			case FieldType::UInt32:			return EPOCH_CACHED_CLASS("System.UInt32")->monoClass;
			case FieldType::UInt64:			return EPOCH_CACHED_CLASS("System.UInt64")->monoClass;
			case FieldType::Float:			return EPOCH_CACHED_CLASS("System.Single")->monoClass;
			case FieldType::Double:			return EPOCH_CACHED_CLASS("System.Double")->monoClass;
			case FieldType::String:			return EPOCH_CACHED_CLASS("System.String")->monoClass;
			case FieldType::AssetHandle:	return EPOCH_CACHED_CLASS("Epoch.AssetHandle")->monoClass;
			case FieldType::LayerMask:		return EPOCH_CACHED_CLASS("Epoch.LayerMask")->monoClass;
			case FieldType::Scene:			return EPOCH_CACHED_CLASS("Epoch.Scene")->monoClass;
			case FieldType::Vector2:		return EPOCH_CACHED_CLASS("Epoch.Vector2")->monoClass;
			case FieldType::Vector3:		return EPOCH_CACHED_CLASS("Epoch.Vector3")->monoClass;
			case FieldType::Color:			return EPOCH_CACHED_CLASS("Epoch.Color")->monoClass;
			case FieldType::Prefab:			return EPOCH_CACHED_CLASS("Epoch.Prefab")->monoClass;
			case FieldType::Entity:			return EPOCH_CACHED_CLASS("Epoch.Entity")->monoClass;
			case FieldType::Material:		return EPOCH_CACHED_CLASS("Epoch.Material")->monoClass;
			case FieldType::Mesh:			return EPOCH_CACHED_CLASS("Epoch.Mesh")->monoClass;
			case FieldType::Texture2D:		return EPOCH_CACHED_CLASS("Epoch.Texture2D")->monoClass;
		}

		return nullptr;
	}
	
	ManagedMethod* ScriptCache::GetManagedMethod(ManagedClass* aManagedClass, const std::string& aName, bool aIgnoreParent)
	{
		if (staticCache == nullptr)
		{
			return nullptr;
		}

		if (aManagedClass == nullptr)
		{
			LOG_ERROR_TAG("ScriptEngine", "Attempting to get method {} from a nullptr class!", aName);
			return nullptr;
		}

		uint32_t methodID = (uint32_t)Hash::GenerateFNVHash(fmt::format("{}:{}", aManagedClass->fullName, aName));
		if (staticCache->methods.find(methodID) != staticCache->methods.end())
		{
			return &staticCache->methods.at(methodID)[0];
		}

		if (!aIgnoreParent && aManagedClass->parentID != 0)
		{
			return GetManagedMethod(&staticCache->classes.at(aManagedClass->parentID), aName);
		}

		LOG_WARNING_TAG("ScriptEngine", "Failed to find method with name: {} in class {}", aName, aManagedClass->fullName);
		return nullptr;
	}

	ManagedMethod* ScriptCache::GetSpecificManagedMethod(ManagedClass* aManagedClass, const std::string& aName, uint32_t aParameterCount, bool aIgnoreParent)
	{
		if (staticCache == nullptr)
		{
			return nullptr;
		}

		if (aManagedClass == nullptr)
		{
			LOG_ERROR_TAG("ScriptEngine", "Attempting to get method {} from a nullptr class!", aName);
			return nullptr;
		}

		ManagedMethod* method = nullptr;

		uint32_t methodID = (uint32_t)Hash::GenerateFNVHash(aManagedClass->fullName + ":" + aName);
		if (staticCache->methods.find(methodID) != staticCache->methods.end())
		{
			for (auto& methodCandiate : staticCache->methods.at(methodID))
			{
				if (methodCandiate.parameterCount == aParameterCount)
				{
					method = &methodCandiate;
					break;
				}
			}
		}

		if (method == nullptr && !aIgnoreParent && aManagedClass->parentID != 0)
		{
			method = GetSpecificManagedMethod(&staticCache->classes.at(aManagedClass->parentID), aName, aParameterCount);
		}

		if (method == nullptr)
		{
			LOG_WARNING_TAG("ScriptEngine", "Failed to find method with name: {} and parameter count: {} in class {}", aName, aParameterCount, aManagedClass->fullName);
		}

		return method;
	}

	void ScriptCache::CacheClass(std::string_view aClassName, MonoClass* aMonoClass)
	{
		EPOCH_PROFILE_FUNC();
		
		MonoType* classType = mono_class_get_type(aMonoClass);
		ManagedClass managedClass;
		managedClass.fullName = aClassName;
		managedClass.id = (uint32_t)Hash::GenerateFNVHash(managedClass.fullName);

		int alignment = 0;
		managedClass.size = mono_type_size(classType, &alignment);
		managedClass.monoClass = aMonoClass;
		staticCache->classes[managedClass.id] = managedClass;
		if (managedClass.fullName.find("Epoch.") != std::string::npos)
		{
			std::shared_ptr<AssemblyInfo> coreAssembly = ScriptEngine::GetCoreAssemblyInfo();
			ScriptCache::CacheClassMethods(coreAssembly, managedClass);
			ScriptCache::CacheClassFields(coreAssembly, managedClass);
			ScriptCache::CacheClassProperties(coreAssembly, managedClass);
		}
	}

	void ScriptCache::CacheClassMethods(std::shared_ptr<AssemblyInfo> aAssemblyInfo, ManagedClass& aManagedClass)
	{
		MonoMethod* monoMethod = nullptr;
		void* methodPtr = 0;
		while ((monoMethod = mono_class_get_methods(aManagedClass.monoClass, &methodPtr)) != NULL)
		{
			MonoMethodSignature* sig = mono_method_signature(monoMethod);

			const uint32_t flags = mono_method_get_flags(monoMethod, nullptr);
			char* fullName = mono_method_full_name(monoMethod, false);

			ManagedMethod method;
			method.id = (uint32_t)Hash::GenerateFNVHash(fullName);
			method.fullName = fullName;
			method.isStatic = flags & MONO_METHOD_ATTR_STATIC;
			method.isVirtual = flags & MONO_METHOD_ATTR_VIRTUAL;
			method.parameterCount = mono_signature_get_param_count(sig);
			method.method = monoMethod;

			staticCache->methods[method.id].push_back(method);

			mono_free(fullName);
		}
		
		if (staticCache->methods.find((uint32_t)Hash::GenerateFNVHash(aManagedClass.fullName + ":OnUpdate")) != staticCache->methods.end())
		{
			aManagedClass.shouldUpdate = true;
		}
		if (staticCache->methods.find((uint32_t)Hash::GenerateFNVHash(aManagedClass.fullName + ":OnLateUpdate")) != staticCache->methods.end())
		{
			aManagedClass.shouldLateUpdate = true;
		}
		if (staticCache->methods.find((uint32_t)Hash::GenerateFNVHash(aManagedClass.fullName + ":OnFixedUpdate")) != staticCache->methods.end())
		{
			aManagedClass.shouldFixedUpdate = true;
		}
	}

	void ScriptCache::CacheClassFields(std::shared_ptr<AssemblyInfo> aAssemblyInfo, ManagedClass& aManagedClass)
	{
		MonoClass* currentClass = aManagedClass.monoClass;
		while (currentClass != nullptr)
		{
			const std::string className = mono_class_get_name(currentClass);
			const std::string classNameSpace = mono_class_get_namespace(currentClass);

			if (classNameSpace.find("Epoch") != std::string::npos && className.find("Entity") != std::string::npos)
			{
				currentClass = nullptr;
				continue;
			}

			MonoClassField* field = nullptr;
			void* fieldPtr = 0;
			while ((field = mono_class_get_fields(currentClass, &fieldPtr)) != NULL)
			{
				const std::string name = mono_field_get_name(field);

				// Properties have a backing field called <PropertyName>k__BackingField. We don't want to include those in the class fields list.
				if (name.find("k__BackingField") != std::string::npos)
				{
					continue;
				}

				MonoType* monoType = mono_field_get_type(field);
				FieldType fieldType = ScriptUtils::GetFieldTypeFromMonoType(monoType);

				if (fieldType == FieldType::Void)
				{
					continue;
				}

				MonoCustomAttrInfo* attributes = mono_custom_attrs_from_field(currentClass, field);

				const uint32_t fieldID = (uint32_t)Hash::GenerateFNVHash(fmt::format("{}:{}", aManagedClass.fullName, name));

				const int32_t typeEncoding = mono_type_get_type(monoType);

				FieldInfo& managedField = staticCache->fields[fieldID];
				managedField.name = name;
				managedField.id = fieldID;
				managedField.type = fieldType;
				managedField.isProperty = false;

				if (typeEncoding == MONO_TYPE_ARRAY || typeEncoding == MONO_TYPE_SZARRAY)
				{
					managedField.flags |= (uint64_t)FieldFlag::IsArray;
				}

				uint32_t visibility = mono_field_get_flags(field) & MONO_FIELD_ATTR_FIELD_ACCESS_MASK;
				switch (visibility)
				{
					case MONO_FIELD_ATTR_PUBLIC:
					{
						managedField.flags &= ~(uint64_t)FieldFlag::Protected;
						managedField.flags &= ~(uint64_t)FieldFlag::Private;
						managedField.flags &= ~(uint64_t)FieldFlag::Internal;
						managedField.flags |= (uint64_t)FieldFlag::Public;
						break;
					}
					case MONO_FIELD_ATTR_FAMILY:
					{
						managedField.flags &= ~(uint64_t)FieldFlag::Public;
						managedField.flags &= ~(uint64_t)FieldFlag::Private;
						managedField.flags &= ~(uint64_t)FieldFlag::Internal;
						managedField.flags |= (uint64_t)FieldFlag::Protected;
						break;
					}
					case MONO_FIELD_ATTR_ASSEMBLY:
					{
						managedField.flags &= ~(uint64_t)FieldFlag::Public;
						managedField.flags &= ~(uint64_t)FieldFlag::Protected;
						managedField.flags &= ~(uint64_t)FieldFlag::Private;
						managedField.flags |= (uint64_t)FieldFlag::Internal;
						break;
					}
					case MONO_FIELD_ATTR_PRIVATE:
					{
						managedField.flags &= ~(uint64_t)FieldFlag::Public;
						managedField.flags &= ~(uint64_t)FieldFlag::Protected;
						managedField.flags &= ~(uint64_t)FieldFlag::Internal;
						managedField.flags |= (uint64_t)FieldFlag::Private;
						break;
					}
				}

				if (attributes && mono_custom_attrs_has_attr(attributes, GetManagedClassByName("Epoch.ShowInEditorAttribute")->monoClass))
				{
					managedField.flags &= ~(uint64_t)FieldFlag::Protected;
					managedField.flags &= ~(uint64_t)FieldFlag::Internal;
					managedField.flags &= ~(uint64_t)FieldFlag::Private;
					managedField.flags |= (uint64_t)FieldFlag::Public;

					MonoObject* attrib = mono_custom_attrs_get_attr(attributes, GetManagedClassByName("Epoch.ShowInEditorAttribute")->monoClass);

					CSharpInstanceInspector inspector(attrib);
					const bool isReadOnly = inspector.GetFieldValue<bool>("readOnly");

#ifndef _RUNTIME
					managedField.tooltip = inspector.GetFieldValue<std::string>("tooltip");
#endif


					if (isReadOnly)
					{
						managedField.flags |= (uint64_t)FieldFlag::ReadOnly;
					}
				}

				if (attributes && mono_custom_attrs_has_attr(attributes, GetManagedClassByName("Epoch.HideFromEditorAttribute")->monoClass))
				{
					managedField.flags &= ~(uint64_t)FieldFlag::Protected;
					managedField.flags &= ~(uint64_t)FieldFlag::Internal;
					managedField.flags &= ~(uint64_t)FieldFlag::Public;
					managedField.flags |= (uint64_t)FieldFlag::Private;
				}
				
#ifndef _RUNTIME
				if (attributes && mono_custom_attrs_has_attr(attributes, GetManagedClassByName("Epoch.SpacingAttribute")->monoClass))
				{
					MonoObject* attrib = mono_custom_attrs_get_attr(attributes, GetManagedClassByName("Epoch.SpacingAttribute")->monoClass);

					CSharpInstanceInspector inspector(attrib);
					managedField.spacing = inspector.GetFieldValue<uint32_t>("spacing");
				}

				if (attributes && mono_custom_attrs_has_attr(attributes, GetManagedClassByName("Epoch.HeaderAttribute")->monoClass))
				{
					MonoObject* attrib = mono_custom_attrs_get_attr(attributes, GetManagedClassByName("Epoch.HeaderAttribute")->monoClass);

					CSharpInstanceInspector inspector(attrib);
					managedField.header = inspector.GetFieldValue<std::string>("header");
				}
#endif

				if (managedField.IsArray())
				{
					MonoClass* fieldArrayClass = mono_type_get_class(monoType);
					MonoClass* elementClass = mono_class_get_element_class(fieldArrayClass);
					MonoType* elementType = mono_class_get_type(elementClass);

					int align;
					managedField.size = mono_type_size(elementType, &align);
				}
				else
				{
					int align;
					managedField.size = mono_type_size(monoType, &align);
				}

				aManagedClass.size += managedField.size;

				if (std::find(aManagedClass.fields.begin(), aManagedClass.fields.end(), fieldID) == aManagedClass.fields.end())
				{
					aManagedClass.fields.push_back(managedField.id);
				}
			}

			currentClass = mono_class_get_parent(currentClass);
		}
	}

	void ScriptCache::CacheClassProperties(std::shared_ptr<AssemblyInfo> aAssemblyInfo, ManagedClass& aManagedClass)
	{
		MonoProperty* prop = nullptr;
		void* propertyIter = 0;
		while ((prop = mono_class_get_properties(aManagedClass.monoClass, &propertyIter)) != NULL)
		{
			std::string name = mono_property_get_name(prop);
			uint32_t propertyID = (uint32_t)Hash::GenerateFNVHash(fmt::format("{}:{}", aManagedClass.fullName, name));

			uint64_t propertyFlags = 0;
			MonoType* monoType = nullptr;

			MonoMethod* propertyGetter = mono_property_get_get_method(prop);
			if (propertyGetter != nullptr)
			{
				MonoMethodSignature* sig = mono_method_signature(propertyGetter);
				monoType = mono_signature_get_return_type(sig);

				uint32_t flags = mono_method_get_flags(propertyGetter, nullptr);
				uint32_t visibility = flags & MONO_METHOD_ATTR_ACCESS_MASK;

				switch (visibility)
				{
					case MONO_FIELD_ATTR_PUBLIC:
					{
						propertyFlags &= ~(uint64_t)FieldFlag::Protected;
						propertyFlags &= ~(uint64_t)FieldFlag::Private;
						propertyFlags &= ~(uint64_t)FieldFlag::Internal;
						propertyFlags |= (uint64_t)FieldFlag::Public;
						break;
					}
					case MONO_FIELD_ATTR_FAMILY:
					{
						propertyFlags &= ~(uint64_t)FieldFlag::Public;
						propertyFlags &= ~(uint64_t)FieldFlag::Private;
						propertyFlags &= ~(uint64_t)FieldFlag::Internal;
						propertyFlags |= (uint64_t)FieldFlag::Protected;
						break;
					}
					case MONO_FIELD_ATTR_ASSEMBLY:
					{
						propertyFlags &= ~(uint64_t)FieldFlag::Public;
						propertyFlags &= ~(uint64_t)FieldFlag::Protected;
						propertyFlags &= ~(uint64_t)FieldFlag::Private;
						propertyFlags |= (uint64_t)FieldFlag::Internal;
						break;
					}
					case MONO_FIELD_ATTR_PRIVATE:
					{
						propertyFlags &= ~(uint64_t)FieldFlag::Public;
						propertyFlags &= ~(uint64_t)FieldFlag::Protected;
						propertyFlags &= ~(uint64_t)FieldFlag::Internal;
						propertyFlags |= (uint64_t)FieldFlag::Private;
						break;
					}
				}

				if ((flags & MONO_METHOD_ATTR_STATIC) != 0)
				{
					propertyFlags |= (uint64_t)FieldFlag::Static;
				}
			}

			MonoMethod* propertySetter = mono_property_get_set_method(prop);
			if (propertySetter != nullptr)
			{
				void* paramIter = nullptr;
				MonoMethodSignature* sig = mono_method_signature(propertySetter);
				monoType = mono_signature_get_params(sig, &paramIter);

				uint32_t flags = mono_method_get_flags(propertySetter, nullptr);
				if ((flags & MONO_METHOD_ATTR_ACCESS_MASK) == MONO_METHOD_ATTR_PRIVATE)
				{
					propertyFlags |= (uint64_t)FieldFlag::ReadOnly;
				}

				if ((flags & MONO_METHOD_ATTR_STATIC) != 0)
				{
					propertyFlags |= (uint64_t)FieldFlag::Static;
				}
			}

			if (propertySetter == nullptr && propertyGetter != nullptr)
			{
				propertyFlags |= (uint64_t)FieldFlag::ReadOnly;
			}

			if (monoType == nullptr)
			{
				LOG_ERROR_TAG("ScriptEngine", "Failed to retrieve managed type for property '{}'", name);
				continue;
			}

			MonoCustomAttrInfo* attributes = mono_custom_attrs_from_property(aManagedClass.monoClass, prop);

			if (!TypeUtils::ContainsAttribute(attributes, "Epoch.ShowInEditorAttribute"))
			{
				continue;
			}

			FieldInfo& managedProperty = staticCache->fields[propertyID];
			managedProperty.name = name;
			managedProperty.id = propertyID;
			managedProperty.type = ScriptUtils::GetFieldTypeFromMonoType(monoType);
			managedProperty.isProperty = true;

			int align;
			managedProperty.size = mono_type_size(monoType, &align);
			aManagedClass.size += managedProperty.size;
			if (std::find(aManagedClass.fields.begin(), aManagedClass.fields.end(), propertyID) == aManagedClass.fields.end())
			{
				aManagedClass.fields.push_back(managedProperty.id);
			}
		}
	}


	const std::unordered_map<uint32_t, ManagedClass>& ScriptCache::GetCachedClasses() { return staticCache->classes; }
	const std::unordered_map<uint32_t, FieldInfo>& ScriptCache::GetCachedFields() { return staticCache->fields; }
	const std::unordered_map<uint32_t, std::vector<ManagedMethod>>& ScriptCache::GetCachedMethods() { return staticCache->methods; }
}
