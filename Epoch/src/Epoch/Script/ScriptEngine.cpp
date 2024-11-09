#include "epch.h"
#include "ScriptEngine.h"
#include <stack>
#include <CommonUtilities/Timer.h>

#include <mono/jit/jit.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/mono-debug.h>

#include "Epoch/Editor/EditorSettings.h"
#include "ScriptGlue.h"
#include "Epoch/Scene/Scene.h"
#include "Epoch/Scene/SceneRenderer.h"
#include "Epoch/Assets/AssetManager.h"
#include "ScriptAsset.h"
#include "ScriptBuilder.h"
#include "ScriptUtils.h"

namespace Epoch
{
	struct ScriptEngineData
	{
		ScriptEngineConfig config;

		MonoDomain* rootDomain = nullptr;
		MonoDomain* scriptsDomain = nullptr;
		MonoDomain* newScriptsDomain = nullptr;
		
		std::shared_ptr<AssemblyInfo> coreAssemblyInfo = nullptr;
		std::shared_ptr<AssemblyInfo> appAssemblyInfo = nullptr;
		MonoAssembly* oldCoreAssembly = nullptr;
		
		std::unordered_map<AssemblyMetadata, MonoAssembly*> referencedAssemblies;

		bool isMonoInitialized = false;
		bool postLoadCleanup = false;
		bool reloadAppAssembly = false;

		std::shared_ptr<Scene> sceneContext = nullptr;
		std::shared_ptr<SceneRenderer> sceneRenderer = nullptr;
		
		MonoClass* timeClass = nullptr;
		MonoMethod* updateDeltaTimeMethod = nullptr;
		MonoMethod* updateUnscaledDeltaTimeMethod = nullptr;
		MonoMethod* updateFixedDeltaTimeMethod = nullptr;
		
		//ScriptEntityMap scriptEntities;
		std::vector<UUID> scriptEntities;
		ScriptInstanceMap scriptInstances;
		std::stack<Entity> runtimeDuplicatedScriptEntities;

		std::unordered_map<UUID, std::unordered_map<uint32_t, std::shared_ptr<FieldStorageBase>>> fieldMap;
		
		std::unique_ptr<filewatch::FileWatch<std::wstring>> watcherHandle = nullptr;
	};

	static ScriptEngineData* staticData = nullptr;

	void ScriptEngine::Init(const ScriptEngineConfig& aConfig)
	{
		EPOCH_PROFILE_FUNC();

		EPOCH_ASSERT(!staticData, "[ScriptEngine]: Trying to call ScriptEngine::Init multiple times!");
		staticData = new ScriptEngineData();

		staticData->config = aConfig;
		staticData->coreAssemblyInfo = std::make_shared<AssemblyInfo>();
		staticData->appAssemblyInfo = std::make_shared<AssemblyInfo>();

		InitMono();
		GCManager::Init();

		if (!LoadCoreAssembly())
		{
			//EPOCH_ASSERT(false, "[ScriptEngine]: Failed to load Epoch C# core!");
			LOG_INFO_TAG("ScriptEngine", "Failed to load Epoch C# core, attempting to build automatically using MSBuild");

			auto scriptCoreAssemblyFile = std::filesystem::current_path().parent_path() / "Epoch-ScriptCore" / "Epoch-ScriptCore.csproj";
			ScriptBuilder::BuildCSProject(scriptCoreAssemblyFile);

			if (!LoadCoreAssembly())
			{
				EPOCH_ASSERT(false, "[ScriptEngine]: Failed to load Epoch C# core!");
			}
		}
	}

	void ScriptEngine::Shutdown()
	{
		EPOCH_PROFILE_FUNC();

		if (staticData->sceneContext)
		{
			for (auto& entityID : staticData->scriptEntities)
			{
				ShutdownScriptEntity(staticData->sceneContext->TryGetEntityWithUUID(entityID));
			}
		}

		staticData->scriptEntities.clear();
		staticData->fieldMap.clear();

		staticData->sceneContext = nullptr;

		ShutdownMono();

		delete staticData;
		staticData = nullptr;
	}

	void ScriptEngine::InitializeRuntime(bool aSkipInitializedEntities)
	{
		EPOCH_PROFILE_FUNC();
		
		EPOCH_ASSERT(staticData->sceneContext, "Tring to initialize script runtime without setting the scene context!");
		EPOCH_ASSERT(staticData->sceneContext->IsPlaying(), "Don't call ScriptEngine::InitializeRuntime if the scene isn't being played!");

		auto view = staticData->sceneContext->GetAllEntitiesWith<ScriptComponent>();
		for (auto enttID : view)
		{
			Entity entity{ enttID, staticData->sceneContext.get()};
			RuntimeInitializeScriptEntity(entity, aSkipInitializedEntities);
		}
	}

	void ScriptEngine::ShutdownRuntime()
	{
		EPOCH_PROFILE_FUNC();
		
		EPOCH_ASSERT(staticData->sceneContext, "Tring to shut down script runtime without having a scene context!");

		for (auto it = staticData->scriptInstances.begin(); it != staticData->scriptInstances.end(); )
		{
			Entity entity = staticData->sceneContext->TryGetEntityWithUUID(it->first);

			if (!entity)
			{
				it++;
				continue;
			}

			ShutdownRuntimeInstance(entity);
			
			it = staticData->scriptInstances.erase(it);
		}

		while (staticData->runtimeDuplicatedScriptEntities.size() > 0)
		{
			staticData->runtimeDuplicatedScriptEntities.pop();
		}

		GCManager::CollectGarbage();
	}

	bool ScriptEngine::LoadAppAssembly()
	{
		EPOCH_PROFILE_FUNC();
		
		if (!FileSystem::Exists(Project::GetScriptModuleFilePath()))
		{
			LOG_ERROR_TAG("ScriptEngine", "Failed to load app assembly! Invalid filepath '{}'", Project::GetScriptModuleFilePath().string());
			return false;
		}

		if (staticData->appAssemblyInfo->assembly)
		{
			staticData->appAssemblyInfo->referencedAssemblies.clear();
			staticData->appAssemblyInfo->assembly = nullptr;
			staticData->appAssemblyInfo->assemblyImage = nullptr;
			
			staticData->referencedAssemblies.clear();

			if (!LoadCoreAssembly())
			{
				return false;
			}
		}

		auto appAssembly = LoadMonoAssembly(Project::GetScriptModuleFilePath());

		if (appAssembly == nullptr)
		{
			LOG_ERROR_TAG("ScriptEngine", "Failed to load app assembly!");
			return false;
		}

		staticData->appAssemblyInfo->filePath = Project::GetScriptModuleFilePath();
		staticData->appAssemblyInfo->assembly = appAssembly;
		staticData->appAssemblyInfo->assemblyImage = mono_assembly_get_image(staticData->appAssemblyInfo->assembly);
		staticData->appAssemblyInfo->classes.clear();
		staticData->appAssemblyInfo->isCoreAssembly = false;
		staticData->appAssemblyInfo->metadata = GetMetadataForImage(staticData->appAssemblyInfo->assemblyImage);

		if (staticData->postLoadCleanup)
		{
			mono_domain_unload(staticData->scriptsDomain);

			if (staticData->oldCoreAssembly)
			{
				staticData->oldCoreAssembly = nullptr;
			}

			staticData->scriptsDomain = staticData->newScriptsDomain;
			staticData->newScriptsDomain = nullptr;
			staticData->postLoadCleanup = false;
		}

		staticData->appAssemblyInfo->referencedAssemblies = GetReferencedAssembliesMetadata(staticData->appAssemblyInfo->assemblyImage);
		auto coreMetadataIt = std::find_if(staticData->appAssemblyInfo->referencedAssemblies.begin(), staticData->appAssemblyInfo->referencedAssemblies.end(), [](const AssemblyMetadata& aMetadata)
		{
			return aMetadata.name == "Epoch-ScriptCore";
		});

		if (coreMetadataIt == staticData->appAssemblyInfo->referencedAssemblies.end())
		{
			LOG_ERROR_TAG("ScriptEngine", "C# project doesn't reference Epoch-ScriptCore?");
			return false;
		}
		
		const auto& coreMetadata = staticData->coreAssemblyInfo->metadata;

		if (coreMetadataIt->majorVersion != coreMetadata.majorVersion || coreMetadataIt->minorVersion != coreMetadata.minorVersion)
		{
			LOG_ERROR("C# project referencing an incompatible script core version!");
			LOG_ERROR("Expected version: {}.{}, referenced version: {}.{}",
				coreMetadata.majorVersion, coreMetadata.minorVersion, coreMetadataIt->majorVersion, coreMetadataIt->minorVersion);

			return false;
		}

		LoadReferencedAssemblies(staticData->appAssemblyInfo);

		LOG_INFO_TAG("ScriptEngine", "Successfully loaded app assembly from: {0}", staticData->appAssemblyInfo->filePath.string());

		ScriptGlue::RegisterGlue();
		ScriptCache::GenerateCacheForAssembly(staticData->appAssemblyInfo);
		return true;
	}

	bool ScriptEngine::ReloadAppAssembly(const bool aScheduleReload)
	{
		if (aScheduleReload)
		{
			staticData->reloadAppAssembly = true;
			return false;
		}
		
		EPOCH_PROFILE_FUNC();

		LOG_INFO_TAG("ScriptEngine", "Reloading {}", Project::GetScriptModuleFilePath().string());

		// Cache old field values and destroy all previous script instances
		std::unordered_map<UUID, std::unordered_map<uint32_t, Buffer>> oldFieldValues;
		
		for (const auto& entityID : staticData->scriptEntities)
		{
			Entity entity = staticData->sceneContext->TryGetEntityWithUUID(entityID);
			
			if (!entity)
			{
				continue;
			}

			if (!entity.HasComponent<ScriptComponent>())
			{
				continue;
			}

			const auto sc = entity.GetComponent<ScriptComponent>();
			oldFieldValues[entityID] = std::unordered_map<uint32_t, Buffer>();
			
			for (auto fieldID : sc.fieldIDs)
			{
				std::shared_ptr<FieldStorageBase> storage = staticData->fieldMap[entityID][fieldID];
			
				if (!storage)
				{
					continue;
				}
			
				const FieldInfo* fieldInfo = storage->GetFieldInfo();
			
				if (!fieldInfo->IsWritable())
				{
					continue;
				}
			
				oldFieldValues[entityID][fieldID] = Buffer::Copy(storage->GetValueBuffer());
			}

			ShutdownScriptEntity(entity, false);

			entity.GetComponent<ScriptComponent>().fieldIDs.clear();
		}

		staticData->scriptEntities.clear();

		bool loaded = LoadAppAssembly();

		
		for (auto& [entityID, fieldMap] : oldFieldValues)
		{
			Entity entity = staticData->sceneContext->GetEntityWithUUID(entityID);
			InitializeScriptEntity(entity);
		
			const auto& sc = entity.GetComponent<ScriptComponent>();
		
			for (auto& [fieldID, fieldValue] : fieldMap)
			{
				std::shared_ptr<FieldStorageBase> storage = staticData->fieldMap[entityID][fieldID];
		
				if (!storage)
				{
					continue;
				}
		
				storage->SetValueBuffer(fieldValue);
				fieldValue.Release();
			}
		}

		GCManager::CollectGarbage();
		LOG_INFO_TAG("ScriptEngine", "Done!");

		if (staticData->sceneContext->IsPlaying())
		{
			InitializeRuntime(false);
		}

		staticData->reloadAppAssembly = false;
		return loaded;
	}

	bool ScriptEngine::ShouldReloadAppAssembly()
	{
		return staticData->reloadAppAssembly;
	}

	void ScriptEngine::UnloadAppAssembly()
	{
		EPOCH_PROFILE_FUNC();
		
		for (auto entityID : staticData->scriptEntities)
		{
			ShutdownScriptEntity(staticData->sceneContext->TryGetEntityWithUUID(entityID));
		}
		staticData->scriptEntities.clear();
		staticData->scriptInstances.clear();

		ScriptCache::ClearCache();
		UnloadAssembly(staticData->appAssemblyInfo);
		ScriptCache::CacheCoreClasses();
		GCManager::CollectGarbage();
	}

	void ScriptEngine::SetSceneContext(const std::shared_ptr<Scene>& aScene, const std::shared_ptr<SceneRenderer>& aSceneRenderer)
	{
		staticData->sceneContext = aScene;
		staticData->sceneRenderer = aSceneRenderer;
	}

	std::shared_ptr<Scene> ScriptEngine::GetSceneContext()
	{
		return staticData->sceneContext;
	}

	std::shared_ptr<SceneRenderer> ScriptEngine::GetSceneRenderer()
	{
		return staticData->sceneRenderer;
	}

	void ScriptEngine::InitializeScriptEntity(Entity aEntity)
	{
		if (!aEntity.HasComponent<ScriptComponent>()) return;

		auto& sc = aEntity.GetComponent<ScriptComponent>();
		if (!IsModuleValid(sc.scriptClassHandle))
		{
			LOG_ERROR_TAG("ScriptEngine", "Tried to initialize script entity with an invalid script!");
			return;
		}

		UUID entityID = aEntity.GetUUID();

		sc.fieldIDs.clear();

		ManagedClass* managedClass = ScriptCache::GetManagedClassByID(GetScriptClassIDFromComponent(sc));
		if (!managedClass)
		{
			return;
		}

		for (auto fieldID : managedClass->fields)
		{
			FieldInfo* fieldInfo = ScriptCache::GetFieldByID(fieldID);
		
			if (!fieldInfo->HasFlag(FieldFlag::Public))
			{
				continue;
			}
			
			if (fieldInfo->IsArray())
			{
				staticData->fieldMap[entityID][fieldID] = std::make_shared<ArrayFieldStorage>(fieldInfo);
			}
			else
			{
				staticData->fieldMap[entityID][fieldID] = std::make_shared<FieldStorage>(fieldInfo);
			}
		
			sc.fieldIDs.push_back(fieldID);
		}

		if (std::find(staticData->scriptEntities.begin(), staticData->scriptEntities.end(), entityID) != staticData->scriptEntities.end())
		{
			//LOG_WARNING_TAG("ScriptEngine", "Initializing a script entity that already is initialized!");
			return;
		}
		staticData->scriptEntities.push_back(entityID);
	}

	void ScriptEngine::RuntimeInitializeScriptEntity(Entity aEntity, bool aSkipInitializedEntities)
	{
		if (!aEntity.HasComponent<ScriptComponent>())
		{
			LOG_ERROR_TAG("ScriptEngine", "Trying to instantiate a script on an entity that doesn't have a ScriptComponent");
			return;
		}
		
		auto scriptComponent = aEntity.GetComponent<ScriptComponent>();

		if (aSkipInitializedEntities && scriptComponent.isRuntimeInitialized) return;
		
		if (!IsModuleValid(scriptComponent.scriptClassHandle))
		{
			const auto managedClass = ScriptCache::GetManagedClassByID(GetScriptClassIDFromComponent(scriptComponent));

			std::string className = "Unknown Script";
			if (managedClass != nullptr)
			{
				className = managedClass->fullName;
			}

			LOG_ERROR_TAG("ScriptEngine", "Tried to instantiate an entity ({}) with an invalid C# class '{}'!", aEntity.GetName(), className);
			LOG_ERROR("Tried to instantiate an entity ({}) with an invalid C# class '{}'!", aEntity.GetName(), className);
			return;
		}

		MonoObject* runtimeInstance = CreateManagedObject(GetScriptClassIDFromComponent(scriptComponent), aEntity.GetUUID());
		GCHandle instanceHandle = GCManager::CreateObjectReference(runtimeInstance, false);
		aEntity.GetComponent<ScriptComponent>().managedInstance = instanceHandle;
		staticData->scriptInstances[aEntity.GetUUID()] = instanceHandle;

		for (auto fieldID : scriptComponent.fieldIDs)
		{
			UUID entityID = aEntity.GetUUID();
		
			if (staticData->fieldMap.find(entityID) != staticData->fieldMap.end())
			{
				staticData->fieldMap[entityID][fieldID]->SetRuntimeInstance(instanceHandle);
			}
		}

		CallMethod(instanceHandle, "OnCreate");

		aEntity.GetComponent<ScriptComponent>().isRuntimeInitialized = true;
	}

	void ScriptEngine::ShutdownScriptEntity(Entity aEntity, bool aErase)
	{
		if (!aEntity.HasComponent<ScriptComponent>()) return;

		auto& sc = aEntity.GetComponent<ScriptComponent>();
		UUID entityID = aEntity.GetUUID();

		if (sc.isRuntimeInitialized && sc.managedInstance != nullptr)
		{
			ShutdownRuntimeInstance(aEntity);
			sc.managedInstance = nullptr;
		}

		if (aErase)
		{
			staticData->fieldMap.erase(entityID);
			sc.fieldIDs.clear();

			staticData->scriptEntities.erase(std::remove(staticData->scriptEntities.begin(), staticData->scriptEntities.end(), entityID), staticData->scriptEntities.end());
		}
	}

	void ScriptEngine::ShutdownRuntimeInstance(Entity aEntity)
	{
		if (!aEntity.HasComponent<ScriptComponent>()) return;

		const auto scriptComponent = aEntity.GetComponent<ScriptComponent>();

		CallMethod(scriptComponent.managedInstance, "OnDestroyInternal");

		for (auto fieldID : scriptComponent.fieldIDs)
		{
			std::shared_ptr<FieldStorageBase> fieldStorage = staticData->fieldMap[aEntity.GetUUID()][fieldID];
			fieldStorage->SetRuntimeInstance(nullptr);
		}

		GCManager::ReleaseObjectReference(scriptComponent.managedInstance);
		aEntity.GetComponent<ScriptComponent>().managedInstance = nullptr;
		aEntity.GetComponent<ScriptComponent>().isRuntimeInitialized = false;
	}
	
	void ScriptEngine::DuplicateScriptInstance(Entity aEntity, Entity aTargetEntity)
	{
		if (!aEntity.HasComponent<ScriptComponent>() || !aTargetEntity.HasComponent<ScriptComponent>()) return;

		const auto& srcScriptComp = aEntity.GetComponent<ScriptComponent>();
		auto& dstScriptComp = aTargetEntity.GetComponent<ScriptComponent>();

		if (srcScriptComp.scriptClassHandle != dstScriptComp.scriptClassHandle)
		{
			const auto srcClass = ScriptCache::GetManagedClassByID(GetScriptClassIDFromComponent(srcScriptComp));
			const auto dstClass = ScriptCache::GetManagedClassByID(GetScriptClassIDFromComponent(dstScriptComp));
			LOG_WARNING_TAG("ScriptEngine", "Attempting to duplicate instance of: {} to an instance of: {}", srcClass->fullName, dstClass->fullName);
			return;
		}

		ShutdownScriptEntity(aTargetEntity);
		InitializeScriptEntity(aTargetEntity);
		
		UUID targetEntityID = aTargetEntity.GetUUID();
		UUID srcEntityID = aEntity.GetUUID();

		for (auto fieldID : srcScriptComp.fieldIDs)
		{
			if (staticData->fieldMap.find(srcEntityID) == staticData->fieldMap.end())
			{
				break;
			}
		
			if (staticData->fieldMap.at(srcEntityID).find(fieldID) == staticData->fieldMap.at(srcEntityID).end())
			{
				continue;
			}
		
			staticData->fieldMap[targetEntityID][fieldID]->CopyFrom(staticData->fieldMap[srcEntityID][fieldID]);
		}

		if (staticData->sceneContext && staticData->sceneContext->IsPlaying())
		{
			staticData->runtimeDuplicatedScriptEntities.push(aTargetEntity);
		}
	}
	
	void ScriptEngine::InitializeRuntimeDuplicatedEntities()
	{
		while (staticData->runtimeDuplicatedScriptEntities.size() > 0)
		{
			Entity& entity = staticData->runtimeDuplicatedScriptEntities.top();

			if (!entity)
			{
				staticData->runtimeDuplicatedScriptEntities.pop();
				continue;
			}

			if (!entity.HasComponent<IDComponent>())
			{
				CONSOLE_LOG_ERROR("Trying to initialize an invalid entity!");
				staticData->runtimeDuplicatedScriptEntities.pop();
				continue;
			}

			RuntimeInitializeScriptEntity(entity);
			ScriptEngine::CallMethod(staticData->scriptInstances[entity.GetUUID()], "OnStart");

			staticData->runtimeDuplicatedScriptEntities.pop();
		}
	}

	std::shared_ptr<FieldStorageBase> ScriptEngine::GetFieldStorage(Entity aEntity, uint32_t aFieldID)
	{
		UUID entityID = aEntity.GetUUID();
		if (staticData->fieldMap.find(entityID) == staticData->fieldMap.end())
		{
			return nullptr;
		}

		if (staticData->fieldMap[entityID].find(aFieldID) == staticData->fieldMap[entityID].end())
		{
			return nullptr;
		}

		return staticData->fieldMap.at(entityID).at(aFieldID);
	}

	std::shared_ptr<AssemblyInfo> ScriptEngine::GetCoreAssemblyInfo()
	{
		return staticData->coreAssemblyInfo;
	}

	std::shared_ptr<AssemblyInfo> ScriptEngine::GetAppAssemblyInfo()
	{
		return staticData->appAssemblyInfo;
	}
		
	MonoDomain* ScriptEngine::GetScriptDomain()
	{
		return staticData->scriptsDomain;
	}

	void ScriptEngine::UpdateDeltaTime()
	{
		{
			float deltaTime = CU::Timer::GetDeltaTime();
			void* param = &deltaTime;
			mono_runtime_invoke(staticData->updateUnscaledDeltaTimeMethod, NULL, &param, nullptr);
		}

		{
			float deltaTime = CU::Timer::GetDeltaTime() * GetSceneContext()->GetTimeScale();
			void* param = &deltaTime;
			mono_runtime_invoke(staticData->updateDeltaTimeMethod, NULL, &param, nullptr);
		}
	}

	void ScriptEngine::UpdateFixedDeltaTime(float aTimeStep)
	{
		void* param = &aTimeStep;
		mono_runtime_invoke(staticData->updateFixedDeltaTimeMethod, NULL, &param, nullptr);
	}
	
	void ScriptEngine::OnProjectChanged(std::shared_ptr<Project> aProject)
	{
		auto filepath = Project::GetScriptModulePath();

		if (!FileSystem::Exists(filepath))
		{
			FileSystem::CreateDirectory(filepath);
		}

		staticData->watcherHandle = std::make_unique<filewatch::FileWatch<std::wstring>>(filepath, [](const auto& aFile, filewatch::Event aEventType)
		{
			ScriptEngine::OnAppAssemblyFolderChanged(aFile, aEventType);
		});
	}

	void ScriptEngine::InitMono()
	{
		EPOCH_PROFILE_FUNC();

		if (staticData->isMonoInitialized) return;

		mono_set_dirs("mono/lib", "mono/etc");

		//if (staticData->config.enableDebugging)
		//if (true)
		//{
		//	if (!FileSystem::Exists("logs"))
		//	{
		//		FileSystem::CreateDirectory("logs");
		//	}
		//
		//	//std::string portString = std::to_string(EditorSettings::Get().scriptDebuggerListenPort);
		//	//std::string debuggerAgentArguments = "--debugger-agent=transport=dt_socket,address=127.0.0.1:" + portString + ",server=y,suspend=n,loglevel=3,logfile=logs/MonoDebugger.log";
		//	std::string debuggerAgentArguments = "--debugger-agent=transport=dt_socket,address=127.0.0.1:2550,server=y,suspend=n,loglevel=3,logfile=logs/MonoDebugger.log";
		//
		//	// Enable mono soft debugger
		//	const char* options[2] =
		//	{
		//		debuggerAgentArguments.c_str(),
		//		"--soft-breakpoints"
		//	};
		//
		//	mono_jit_parse_options(2, (char**)options);
		//	mono_debug_init(MONO_DEBUG_FORMAT_MONO);
		//}

		staticData->rootDomain = mono_jit_init("EpochJITRuntime");
		EPOCH_ASSERT(staticData->rootDomain, "[ScriptEngine]: Unable to initialize Mono JIT");
		
		//if (staticData->config.enableDebugging)
		//if (true)
		//{
		//	mono_debug_domain_create(staticData->rootDomain);
		//}

		mono_thread_set_main(mono_thread_current());

		staticData->isMonoInitialized = true;
	}

	uint32_t ScriptEngine::GetScriptClassIDFromComponent(const ScriptComponent& aScriptComponent)
	{
		if (!AssetManager::IsAssetHandleValid(aScriptComponent.scriptClassHandle))
		{
			return 0;
		}

		if (staticData->appAssemblyInfo == nullptr)
		{
			return 0;
		}

		std::shared_ptr<ScriptAsset> scriptAsset = AssetManager::GetAsset<ScriptAsset>(aScriptComponent.scriptClassHandle);
		return scriptAsset->GetClassID();
	}

	bool ScriptEngine::IsModuleValid(AssetHandle aScriptAssetHandle)
	{
		if (!AssetManager::IsAssetHandleValid(aScriptAssetHandle))
		{
			return false;
		}

		if (staticData->appAssemblyInfo == nullptr)
		{
			return false;
		}

		std::shared_ptr<ScriptAsset> scriptAsset = AssetManager::GetAsset<ScriptAsset>(aScriptAssetHandle);
		EPOCH_ASSERT(scriptAsset, "Failed to get script asset!");
		return ScriptCache::GetManagedClassByID(scriptAsset->GetClassID()) != nullptr;
	}

	void ScriptEngine::ShutdownMono()
	{
		EPOCH_PROFILE_FUNC();

		if (!staticData->isMonoInitialized)
		{
			LOG_ERROR_TAG("ScriptEngine", "Trying to shutdown Mono multiple times!");
			return;
		}

		staticData->scriptsDomain = nullptr;
		mono_jit_cleanup(staticData->rootDomain);
		staticData->rootDomain = nullptr;

		staticData->isMonoInitialized = false;
	}

	bool ScriptEngine::LoadCoreAssembly()
	{
		EPOCH_PROFILE_FUNC();
		
		LOG_DEBUG_TAG("ScriptEngine", "Trying to load core assembly: '{}", staticData->config.coreAssemblyPath.string());
		if (!FileSystem::Exists(staticData->config.coreAssemblyPath))
		{
			LOG_ERROR_TAG("ScriptEngine", "Could not find core assembly! Path: ''", staticData->config.coreAssemblyPath.string());
			return false;
		}
		
		std::string domainName = "EpochScriptRuntime";

		if (staticData->scriptsDomain)
		{
			ScriptCache::Shutdown();
			staticData->coreAssemblyInfo->referencedAssemblies.clear();

			staticData->newScriptsDomain = mono_domain_create_appdomain(domainName.data(), nullptr);
			mono_domain_set(staticData->newScriptsDomain, true);
			staticData->postLoadCleanup = true;
		}
		else
		{
			staticData->scriptsDomain = mono_domain_create_appdomain(domainName.data(), nullptr);
			mono_domain_set(staticData->scriptsDomain, true);
			staticData->postLoadCleanup = false;
		}
		
		staticData->oldCoreAssembly = staticData->coreAssemblyInfo->assembly;
		staticData->coreAssemblyInfo->filePath = staticData->config.coreAssemblyPath;
		staticData->coreAssemblyInfo->assembly = LoadMonoAssembly(staticData->config.coreAssemblyPath);

		if (staticData->coreAssemblyInfo->assembly == nullptr)
		{
			LOG_ERROR_TAG("ScriptEngine", "Failed to load core assembly!");
			return false;
		}

		staticData->timeClass = nullptr;
		staticData->updateDeltaTimeMethod = nullptr;
		staticData->updateUnscaledDeltaTimeMethod = nullptr;
		staticData->updateFixedDeltaTimeMethod = nullptr;

		staticData->coreAssemblyInfo->classes.clear();
		staticData->coreAssemblyInfo->assemblyImage = mono_assembly_get_image(staticData->coreAssemblyInfo->assembly);
		staticData->coreAssemblyInfo->isCoreAssembly = true;
		staticData->coreAssemblyInfo->metadata = GetMetadataForImage(staticData->coreAssemblyInfo->assemblyImage);
		staticData->coreAssemblyInfo->referencedAssemblies = GetReferencedAssembliesMetadata(staticData->coreAssemblyInfo->assemblyImage);
		
		LoadReferencedAssemblies(staticData->coreAssemblyInfo);
		
		LOG_INFO_TAG("ScriptEngine", "Successfully loaded core assembly from: '{}'", staticData->config.coreAssemblyPath.string());
		
		ScriptCache::Init();

		staticData->timeClass = mono_class_from_name(GetCoreAssemblyInfo()->assemblyImage, "Epoch", "Time");
		staticData->updateDeltaTimeMethod = mono_class_get_method_from_name(staticData->timeClass, "UpdateDeltaTime", 1);
		staticData->updateUnscaledDeltaTimeMethod = mono_class_get_method_from_name(staticData->timeClass, "UpdateUnscaledDeltaTime", 1);
		staticData->updateFixedDeltaTimeMethod = mono_class_get_method_from_name(staticData->timeClass, "UpdateFixedDeltaTime", 1);

		return true;
	}

	MonoAssembly* ScriptEngine::LoadMonoAssembly(const std::filesystem::path& aAssemblyPath)
	{
		EPOCH_PROFILE_FUNC();
		
		if (!FileSystem::Exists(aAssemblyPath)) return nullptr;

		Buffer fileData = FileSystem::ReadBytes(aAssemblyPath);

		// NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
		MonoImageOpenStatus status;
		MonoImage* image = mono_image_open_from_data_full(fileData.As<char>(), uint32_t(fileData.size), 1, &status, 0);

		fileData.Release();

		if (status != MONO_IMAGE_OK)
		{
			const char* errorMessage = mono_image_strerror(status);
			LOG_ERROR_TAG("ScriptEngine", "Failed to open C# assembly '{}'\n\t\tMessage: {}", aAssemblyPath.string(), errorMessage);
			return nullptr;
		}

		// Load C# debug symbols if debugging is enabled
		//if (staticData->config.enableDebugging)
		//if (true)
		//{
		//	// First check if we have a .dll.pdb file
		//	bool loadDebugSymbols = true;
		//	std::filesystem::path pdbPath = aAssemblyPath.string() + ".pdb";
		//	if (!FileSystem::Exists(pdbPath))
		//	{
		//		// Otherwise try just .pdb
		//		pdbPath = aAssemblyPath;
		//		pdbPath.replace_extension("pdb");
		//
		//		if (!FileSystem::Exists(pdbPath))
		//		{
		//			LOG_WARNING_TAG("ScriptEngine", "Couldn't find .pdb file for assembly {}, no debug symbols will be loaded.", aAssemblyPath.string());
		//			loadDebugSymbols = false;
		//		}
		//	}
		//
		//	if (loadDebugSymbols)
		//	{
		//		LOG_INFO_TAG("ScriptEngine", "Loading debug symbols from '{}'", pdbPath.string());
		//		Buffer buffer = FileSystem::ReadBytes(pdbPath);
		//		mono_debug_open_image_from_memory(image, buffer.As<mono_byte>(), uint32_t(buffer.size));
		//		buffer.Release();
		//	}
		//}

		MonoAssembly* assembly = mono_assembly_load_from_full(image, aAssemblyPath.string().c_str(), &status, 0);
		mono_image_close(image);

		return assembly;
	}

	void ScriptEngine::UnloadAssembly(std::shared_ptr<AssemblyInfo> aAssemblyInfo)
	{
		EPOCH_PROFILE_FUNC();

		aAssemblyInfo->classes.clear();
		aAssemblyInfo->assembly = nullptr;
		aAssemblyInfo->assemblyImage = nullptr;

		if (aAssemblyInfo->isCoreAssembly)
		{
			staticData->coreAssemblyInfo = std::make_shared<AssemblyInfo>();
		}
		else
		{
			staticData->appAssemblyInfo = std::make_shared<AssemblyInfo>();
		}
	}

	void ScriptEngine::LoadReferencedAssemblies(const std::shared_ptr<AssemblyInfo>& aAssemblyInfo)
	{
		EPOCH_PROFILE_FUNC();
		
		static std::filesystem::path staticAssembliesBasePath = std::filesystem::absolute("mono") / "lib" / "mono" / "4.5";

		for (const auto& assemblyMetadata : aAssemblyInfo->referencedAssemblies)
		{
			// Ignore Epoch-ScriptCore and mscorlib, since they're already loaded
			if (assemblyMetadata.name.find("Epoch-ScriptCore") != std::string::npos)
			{
				continue;
			}

			if (assemblyMetadata.name.find("mscorlib") != std::string::npos)
			{
				continue;
			}

			std::filesystem::path assemblyPath = staticAssembliesBasePath / (fmt::format("{0}.dll", assemblyMetadata.name));
			LOG_INFO_TAG("ScriptEngine", "Loading assembly {0} referenced by {1}", assemblyPath.string(), aAssemblyInfo->filePath.filename().string());

			MonoAssembly* assembly = LoadMonoAssembly(assemblyPath);

			if (assembly == nullptr)
			{
				LOG_ERROR_TAG("ScriptEngine", "Failed to load assembly {0} referenced by {1}", assemblyMetadata.name, aAssemblyInfo->filePath);
				continue;
			}

			staticData->referencedAssemblies[assemblyMetadata] = assembly;
		}
	}
	
	AssemblyMetadata ScriptEngine::GetMetadataForImage(MonoImage* aImage)
	{
		EPOCH_PROFILE_FUNC();
		
		AssemblyMetadata metadata;

		const MonoTableInfo* t = mono_image_get_table_info(aImage, MONO_TABLE_ASSEMBLY);
		uint32_t cols[MONO_ASSEMBLY_SIZE];
		mono_metadata_decode_row(t, 0, cols, MONO_ASSEMBLY_SIZE);

		metadata.name = mono_metadata_string_heap(aImage, cols[MONO_ASSEMBLY_NAME]);
		metadata.majorVersion = cols[MONO_ASSEMBLY_MAJOR_VERSION] > 0 ? cols[MONO_ASSEMBLY_MAJOR_VERSION] : 1;
		metadata.minorVersion = cols[MONO_ASSEMBLY_MINOR_VERSION];
		metadata.buildVersion = cols[MONO_ASSEMBLY_BUILD_NUMBER];
		metadata.revisionVersion = cols[MONO_ASSEMBLY_REV_NUMBER];

		return metadata;
	}

	std::vector<AssemblyMetadata> ScriptEngine::GetReferencedAssembliesMetadata(MonoImage* aImage)
	{
		EPOCH_PROFILE_FUNC();
		
		const MonoTableInfo* t = mono_image_get_table_info(aImage, MONO_TABLE_ASSEMBLYREF);
		int rows = mono_table_info_get_rows(t);

		std::vector<AssemblyMetadata> metadata;
		for (int i = 0; i < rows; i++)
		{
			uint32_t cols[MONO_ASSEMBLYREF_SIZE];
			mono_metadata_decode_row(t, i, cols, MONO_ASSEMBLYREF_SIZE);

			auto& assemblyMetadata = metadata.emplace_back();
			assemblyMetadata.name = mono_metadata_string_heap(aImage, cols[MONO_ASSEMBLYREF_NAME]);
			assemblyMetadata.majorVersion = cols[MONO_ASSEMBLYREF_MAJOR_VERSION];
			assemblyMetadata.minorVersion = cols[MONO_ASSEMBLYREF_MINOR_VERSION];
			assemblyMetadata.buildVersion = cols[MONO_ASSEMBLYREF_BUILD_NUMBER];
			assemblyMetadata.revisionVersion = cols[MONO_ASSEMBLYREF_REV_NUMBER];
		}

		return metadata;
	}
	
	MonoObject* ScriptEngine::CreateManagedObject(ManagedClass* aManagedClass)
	{
		MonoObject* monoObject = mono_object_new(staticData->scriptsDomain, aManagedClass->monoClass);
		EPOCH_ASSERT(monoObject, "Failed to create MonoObject!");
		return monoObject;
	}
	
	void ScriptEngine::InitRuntimeObject(MonoObject* aMonoObject)
	{
		mono_runtime_object_init(aMonoObject);
	}

	bool ScriptEngine::IsEntityInstantiated(Entity aEntity, bool aCheckOnCreateCalled)
	{
		if (!staticData->sceneContext || !staticData->sceneContext->IsPlaying())
		{
			return false;
		}

		if (!aEntity.HasComponent<ScriptComponent>())
		{
			return false;
		}

		const auto& scriptComponent = aEntity.GetComponent<ScriptComponent>();

		if (scriptComponent.managedInstance == nullptr)
		{
			return false;
		}

		if (aCheckOnCreateCalled && !scriptComponent.isRuntimeInitialized)
		{
			return false;
		}

		return staticData->scriptInstances.find(aEntity.GetUUID()) != staticData->scriptInstances.end();
	}

	GCHandle ScriptEngine::GetEntityInstance(UUID aEntityID)
	{
		if (staticData->scriptInstances.find(aEntityID) == staticData->scriptInstances.end())
		{
			return nullptr;
		}

		if (staticData->scriptInstances.find(aEntityID) == staticData->scriptInstances.end())
		{
			return nullptr;
		}

		return staticData->scriptInstances.at(aEntityID);
	}

	const std::unordered_map<UUID, GCHandle>& ScriptEngine::GetEntityInstances() { return staticData->scriptInstances; }
	
	void ScriptEngine::CallMethod(MonoObject* aMonoObject, ManagedMethod* aManagedMethod, const void** aParameters)
	{
		MonoObject* exception = NULL;
		mono_runtime_invoke(aManagedMethod->method, aMonoObject, const_cast<void**>(aParameters), &exception);
		ScriptUtils::HandleException(exception);
	}

	void ScriptEngine::OnAppAssemblyFolderChanged(const std::filesystem::path& aFilepath, filewatch::Event aEventType)
	{
		if (!EditorSettings::Get().automaticallyReloadScriptAssembly) return;

		if (aEventType != filewatch::Event::modified) return;
		if (aFilepath.extension().string() != ".dll") return;
		if (aFilepath.filename().string().find(Project::GetActive()->GetConfig().projectFileName.stem().string()) == std::string::npos) return;

		// Schedule assembly reload
		ReloadAppAssembly(true);
	}
}
