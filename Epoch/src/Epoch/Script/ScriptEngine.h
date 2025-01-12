#pragma once
#include "ScriptEngineConfig.h"
#include "ScriptTypes.h"
#include "Epoch/Core/Hash.h"
#include "Epoch/Scene/Scene.h"
#include "Epoch/Project/Project.h"
#include "Epoch/Utils/FileSystem.h"
#include "ScriptCache.h"

extern "C"
{
	typedef struct _MonoDomain MonoDomain;
}

namespace Epoch
{
	using ScriptInstanceMap = std::unordered_map<UUID, GCHandle>;

	class Scene;
	class SceneRenderer;

	class ScriptEngine
	{
	public:
		static void Init(const ScriptEngineConfig& aConfig);
		static void Shutdown();

		static void InitializeRuntime(bool aSkipInitializedEntities = true);
		static void ShutdownRuntime();

		static void UpdateDeltaTime();
		static void UpdateFixedDeltaTime(float aTimeStep);

		static void OnProjectChanged(std::shared_ptr<Project> aProject);

		static bool LoadAppAssembly();
		static bool ReloadAppAssembly(const bool aScheduleReload = false);
		static bool ShouldReloadAppAssembly();
		static void UnloadAppAssembly();

		static void SetSceneContext(const std::shared_ptr<Scene>& aScene, const std::shared_ptr<SceneRenderer>& aSceneRenderer);
		static std::shared_ptr<Scene> GetSceneContext();
		static std::shared_ptr<SceneRenderer> GetSceneRenderer();

		static void InitializeScriptEntity(Entity aEntity);
		static void RuntimeInitializeScriptEntity(Entity aEntity, bool aSkipInitializedEntities = true);
		static void ShutdownScriptEntity(Entity aEntity, bool aErase = true);
		static void ShutdownRuntimeInstance(Entity aEntity);
		static void DuplicateScriptInstance(Entity aEntity, Entity aTargetEntity);
		static void InitializeRuntimeDuplicatedEntities();
		
		static bool IsEntityInstantiated(Entity aEntity, bool aCheckOnCreateCalled = true);
		static GCHandle GetEntityInstance(UUID aEntityID);
		static const std::unordered_map<UUID, GCHandle>& GetEntityInstances();

		static uint32_t GetScriptClassIDFromComponent(const ScriptComponent& aScriptComponent);
		static bool IsModuleValid(AssetHandle aScriptAssetHandle);

		template<typename... TConstructorArgs>
		static MonoObject* CreateManagedObject(const std::string& aClassName, TConstructorArgs&&... aArgs)
		{
			return CreateManagedObject_Internal(ScriptCache::GetManagedClassByID((uint32_t)EPOCH_SCRIPT_CLASS_ID(aClassName)), std::forward<TConstructorArgs>(aArgs)...);
		}

		template<typename... TConstructorArgs>
		static MonoObject* CreateManagedObject(uint32_t aClassID, TConstructorArgs&&... aArgs)
		{
			return CreateManagedObject_Internal(ScriptCache::GetManagedClassByID(aClassID), std::forward<TConstructorArgs>(aArgs)...);
		}

		static std::shared_ptr<FieldStorageBase> GetFieldStorage(Entity aEntity, uint32_t aFieldID);
		
		static std::shared_ptr<AssemblyInfo> GetCoreAssemblyInfo();
		static std::shared_ptr<AssemblyInfo> GetAppAssemblyInfo();

		static MonoDomain* GetScriptDomain();

		template<typename... TArgs>
		static void CallMethod(MonoObject* aManagedObject, const std::string& aMethodName, TArgs&&... aArgs)
		{
			if (aManagedObject == nullptr)
			{
				LOG_WARNING_TAG("ScriptEngine", "Attempting to call method {} on an invalid instance!", aMethodName);
				return;
			}
			
			constexpr size_t argsCount = sizeof...(aArgs);
			
			ManagedClass* managedClass = ScriptCache::GetMonoObjectClass(aManagedObject);
			if (managedClass == nullptr)
			{
				LOG_ERROR_TAG("ScriptEngine", "Failed to find ManagedClass!");
				return;
			}
			
			ManagedMethod* method = ScriptCache::GetSpecificManagedMethod(managedClass, aMethodName, argsCount, true);
			if (method == nullptr)
			{
				//LOG_ERROR_TAG("ScriptEngine", "Failed to find a C# method called {} with {} parameters", aMethodName, argsCount);
				return;
			}
			
			if constexpr (argsCount > 0)
			{
				const void* data[] = { &aArgs... };
				CallMethod(aManagedObject, method, data);
			}
			else
			{
				CallMethod(aManagedObject, method, nullptr);
			}
		}

		template<typename... TArgs>
		static void CallMethod(GCHandle aInstance, const std::string& aMethodName, TArgs&&... aArgs)
		{
			if (aInstance == nullptr)
			{
				LOG_WARNING_TAG("ScriptEngine", "Attempting to call method {} on an invalid instance!", aMethodName);
				return;
			}
			
			CallMethod(GCManager::GetReferencedObject(aInstance), aMethodName, std::forward<TArgs>(aArgs)...);
		}

	private:
		static void InitMono();
		static void ShutdownMono();

		static bool LoadCoreAssembly();
		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& aAssemblyPath);
		static void UnloadAssembly(std::shared_ptr<AssemblyInfo> aAssemblyInfo);

		static void LoadReferencedAssemblies(const std::shared_ptr<AssemblyInfo>& aAssemblyInfo);

		static AssemblyMetadata GetMetadataForImage(MonoImage* aImage);
		static std::vector<AssemblyMetadata> GetReferencedAssembliesMetadata(MonoImage* aImage);

		static MonoObject* CreateManagedObject(ManagedClass* aManagedClass);
		static void InitRuntimeObject(MonoObject* aMonoObject);
		static void CallMethod(MonoObject* aMonoObject, ManagedMethod* aManagedMethod, const void** aParameters);

		static void OnAppAssemblyFolderChanged(const std::filesystem::path& aFilepath, FilewatchEvent aEventType);

		template<typename... TConstructorArgs>
		static MonoObject* CreateManagedObject_Internal(ManagedClass* aManagedClass, TConstructorArgs&&... aArgs)
		{
			if (aManagedClass == nullptr)
			{
				LOG_ERROR_TAG("ScriptEngine", "Attempting to create managed object with a null class!");
				return nullptr;
			}

			if (aManagedClass->isAbstract)
			{
				return nullptr;
			}

			MonoObject* obj = CreateManagedObject(aManagedClass);

			if (aManagedClass->isStruct)
			{
				return obj;
			}

			constexpr size_t argsCount = sizeof...(aArgs);
			ManagedMethod* ctor = ScriptCache::GetSpecificManagedMethod(aManagedClass, ".ctor", argsCount);

			InitRuntimeObject(obj);

			if constexpr (argsCount > 0)
			{
				if (ctor == nullptr)
				{
					LOG_ERROR_TAG("ScriptEngine", "Failed to call constructor with {} parameters for class '{}'.", argsCount, aManagedClass->fullName);
					return obj;
				}

				const void* data[] = { &aArgs... };
				CallMethod(obj, ctor, data);
			}

			return obj;
		}
		
		friend class ScriptCache;
		friend class ScriptUtils;
	};
}
