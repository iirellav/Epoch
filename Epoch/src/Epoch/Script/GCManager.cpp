#include "epch.h"
#include "GCManager.h"

#include <mono/metadata/object.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/profiler.h>

#include "ScriptCache.h"

namespace Epoch
{
	using ReferenceMap = std::unordered_map<GCHandle, MonoObject*>;

	struct GCState
	{
		ReferenceMap strongReferences;
		ReferenceMap weakReferences;
	};
	
	static GCState* staticGCState = nullptr;

	void GCManager::Init()
	{
		EPOCH_ASSERT(!staticGCState, "Trying to initialize GC Manager multiple times!");

		staticGCState = new GCState();
	}

	void GCManager::Shutdown()
	{
		if (staticGCState->strongReferences.size() > 0)
		{
			LOG_ERROR("ScriptEngine", "Memory leak detected!");
			LOG_ERROR("ScriptEngine", "Not all GCHandles have been cleaned up!");

			for (auto[handle, monoObject] : staticGCState->strongReferences)
			{
				mono_gchandle_free_v2(handle);
			}

			staticGCState->strongReferences.clear();
		}

		if (staticGCState->weakReferences.size() > 0)
		{
			LOG_ERROR("ScriptEngine", "Memory leak detected!");
			LOG_ERROR("ScriptEngine", "Not all GCHandles have been cleaned up!");

			for (auto [handle, monoObject] : staticGCState->weakReferences)
			{
				mono_gchandle_free_v2(handle);
			}

			staticGCState->weakReferences.clear();
		}

		// Collect any leftover garbage
		mono_gc_collect(mono_gc_max_generation());
		while (mono_gc_pending_finalizers());

		delete staticGCState;
		staticGCState = nullptr;
	}

	void GCManager::CollectGarbage(bool aBlockUntilFinalized)
	{
		LOG_INFO_TAG("ScriptEngine", "Collecting garbage...");
		mono_gc_collect(mono_gc_max_generation());
		if (aBlockUntilFinalized)
		{
			while (mono_gc_pending_finalizers());
			LOG_INFO_TAG("ScriptEngine", "GC Finished...");
		}
	}

	GCHandle GCManager::CreateObjectReference(MonoObject* aManagedObject, bool aWeakReference, bool aPinned, bool aTrack)
	{
		GCHandle handle = aWeakReference ? mono_gchandle_new_weakref_v2(aManagedObject, aPinned) : mono_gchandle_new_v2(aManagedObject, aPinned);
		EPOCH_ASSERT(handle, "Failed to retrieve valid GC Handle!");

		if (aTrack)
		{
			if (aWeakReference)
			{
				staticGCState->weakReferences[handle] = aManagedObject;
			}
			else
			{
				staticGCState->strongReferences[handle] = aManagedObject;
			}
		}

		return handle;
	}

	bool GCManager::IsHandleValid(GCHandle aHandle)
	{
		if (aHandle == nullptr)
		{
			return false;
		}

		MonoObject* obj = mono_gchandle_get_target_v2(aHandle);

		if (obj == nullptr)
		{
			return false;
		}

		if (mono_object_get_vtable(obj) == nullptr)
		{
			return false;
		}

		return true;
	}

	MonoObject* GCManager::GetReferencedObject(GCHandle aHandle)
	{
		MonoObject* obj = mono_gchandle_get_target_v2(aHandle);
		if (obj == nullptr || mono_object_get_vtable(obj) == nullptr)
		{
			return nullptr;
		}
		return obj;
	}

	void GCManager::ReleaseObjectReference(GCHandle aHandle)
	{
		if (mono_gchandle_get_target_v2(aHandle) != nullptr)
		{
			mono_gchandle_free_v2(aHandle);
		}
		else
		{
			LOG_ERROR_TAG("ScriptEngine", "Tried to release an object reference using an invalid handle!");
			return;
		}

		if (staticGCState->strongReferences.find(aHandle) != staticGCState->strongReferences.end())
		{
			staticGCState->strongReferences.erase(aHandle);
		}

		if (staticGCState->strongReferences.find(aHandle) != staticGCState->strongReferences.end())
		{
			staticGCState->strongReferences.erase(aHandle);
		}
	}
}