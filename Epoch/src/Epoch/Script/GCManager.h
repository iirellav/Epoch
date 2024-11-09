#pragma once
#include "ScriptTypes.h"

extern "C"
{
	typedef struct _MonoObject MonoObject;
}

namespace Epoch
{
	using GCHandle = void*;

	class GCManager
	{
	public:
		static void Init();
		static void Shutdown();

		static void CollectGarbage(bool aBlockUntilFinalized = true);

		static GCHandle CreateObjectReference(MonoObject* aManagedObject, bool aWeakReference, bool aPinned = false, bool aTrack = true);
		static bool IsHandleValid(GCHandle aHandle);
		static MonoObject* GetReferencedObject(GCHandle aHandle);
		static void ReleaseObjectReference(GCHandle aHandle);
	};
}
