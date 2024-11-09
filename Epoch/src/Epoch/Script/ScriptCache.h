#pragma once
#include "ScriptTypes.h"
#include "FieldStorage.h"

#define EPOCH_CORE_CLASS(coreClassName) ScriptCache::GetManagedClassByName("Epoch."#coreClassName)
#define EPOCH_CACHED_CLASS(cachedClassName) ScriptCache::GetManagedClassByName(cachedClassName)
#define EPOCH_CACHED_CLASS_RAW(cashedClassRawName) ScriptCache::GetManagedClassByName(cashedClassRawName)->monoClass
#define EPOCH_CACHED_METHOD(className, method, paramCount) ScriptCache::GetSpecificManagedMethod(ScriptCache::GetManagedClassByName(className), method, paramCount)
//#define EPOCH_CACHED_FIELD(class, field) ScriptCache::GetFieldByName(ScriptCache::GetManagedClassByName(class), field)
//#define EPOCH_CACHED_FIELD_STORAGE(class, field) ScriptCache::GetFieldStorage(ScriptCache::GetFieldByName(ScriptCache::GetManagedClassByName(class), field)->ID)
#define EPOCH_SCRIPT_CLASS_ID(name) Hash::GenerateFNVHash(name)

namespace Epoch
{
	class ScriptCache
	{
	public:
		static void Init();
		static void Shutdown();

		static void CacheCoreClasses();
		static void ClearCache();

		static void GenerateCacheForAssembly(std::shared_ptr<AssemblyInfo> aAssemblyInfo);

		static ManagedClass* GetManagedClassByName(const std::string& aClassName);
		static ManagedClass* GetManagedClassByID(uint32_t aClassID);
		static ManagedClass* GetManagedClass(MonoClass* aMonoClass);
		static ManagedClass* GetMonoObjectClass(MonoObject* aMonoObject);
		static FieldInfo* GetFieldByID(uint32_t aFieldID);
		static FieldInfo* GetFieldByName(const ManagedClass* aManagedClass, const std::string& aFieldName);
		static MonoClass* GetFieldTypeClass(FieldType aFieldType);

		static ManagedMethod* GetManagedMethod(ManagedClass* aManagedClass, const std::string& aName, bool aIgnoreParent = false);
		static ManagedMethod* GetSpecificManagedMethod(ManagedClass* aManagedClass, const std::string& aName, uint32_t aParameterCount, bool aIgnoreParent = false);

	private:
		static void CacheClass(std::string_view aClassName, MonoClass* aMonoClass);
		static void CacheClassMethods(std::shared_ptr<AssemblyInfo> aAssemblyInfo, ManagedClass& aManagedClass);
		static void CacheClassFields(std::shared_ptr<AssemblyInfo> aAssemblyInfo, ManagedClass& aManagedClass);
		static void CacheClassProperties(std::shared_ptr<AssemblyInfo> aAssemblyInfo, ManagedClass& aManagedClass);

		static const std::unordered_map<uint32_t, ManagedClass>& GetCachedClasses();
		static const std::unordered_map<uint32_t, FieldInfo>& GetCachedFields();
		static const std::unordered_map<uint32_t, std::vector<ManagedMethod>>& GetCachedMethods();
	
		friend class ScriptEngineDebugPanel;
	};
}
