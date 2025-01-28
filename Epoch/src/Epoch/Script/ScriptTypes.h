#pragma once
#include <memory>
#include <string>
#include <filesystem>
#include <unordered_map>
#include <vector>

extern "C"
{
	typedef struct _MonoAssembly MonoAssembly;
	typedef struct _MonoImage MonoImage;
	typedef struct _MonoClassField MonoClassField;
	typedef struct _MonoProperty MonoProperty;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoType MonoType;
}

namespace Epoch
{
	namespace Utils
	{
		// NOTE: Stolen from Boost
		template<typename T>
		inline void HashCombine(size_t& result, const T& value)
		{
			std::hash<T> h;
			result ^= h(value) + 0x9e3779b9 + (result << 6) + (result >> 2);
		}
	}

	namespace TypeUtils
	{
		bool ContainsAttribute(void* aAttributeList, const std::string& aAttributeName);
	}

	struct ManagedClass
	{
		uint32_t id = 0;
		std::string fullName = "";
		std::vector<uint32_t> fields;
		std::vector<uint32_t> methods;
		uint32_t size = 0;

		// Will also be true if class is static
		bool isAbstract = false;
		bool isStruct = false;
		bool isEnum = false;

		// Skip calling functions if not overriden
		bool shouldUpdate = false;
		bool shouldLateUpdate = false;
		bool shouldFixedUpdate = false;

		bool shouldDebug = false;

		uint32_t parentID = 0;

		MonoClass* monoClass = nullptr;

		~ManagedClass()
		{
			monoClass = nullptr;
		}
	};

	struct ManagedMethod
	{
		uint32_t id = 0;
		std::string fullName = "";
		bool isVirtual = false;
		bool isStatic = false;
		uint32_t parameterCount = 0;

		MonoMethod* method = nullptr;

		~ManagedMethod()
		{
			method = nullptr;
		}
	};

	struct AssemblyMetadata
	{
		std::string name;
		uint32_t majorVersion;
		uint32_t minorVersion;
		uint32_t buildVersion;
		uint32_t revisionVersion;

		bool operator==(const AssemblyMetadata& aOther) const
		{
			return name == aOther.name && majorVersion == aOther.majorVersion && minorVersion == aOther.minorVersion && buildVersion == aOther.buildVersion && revisionVersion == aOther.revisionVersion;
		}

		bool operator!=(const AssemblyMetadata& other) const { return !(*this == other); }
	};

	struct AssemblyInfo
	{
		std::filesystem::path filePath = "";
		MonoAssembly* assembly = nullptr;
		MonoImage* assemblyImage = nullptr;
		std::vector<uint32_t> classes;
		bool isCoreAssembly = false;
		AssemblyMetadata metadata;
		std::vector<AssemblyMetadata> referencedAssemblies;
	};
}

namespace std
{
	template<>
	struct hash<Epoch::AssemblyMetadata>
	{
		size_t operator()(const Epoch::AssemblyMetadata& aMetadata) const
		{
			size_t result = 0;
			Epoch::Utils::HashCombine(result, aMetadata.name);
			Epoch::Utils::HashCombine(result, aMetadata.majorVersion);
			Epoch::Utils::HashCombine(result, aMetadata.minorVersion);
			Epoch::Utils::HashCombine(result, aMetadata.buildVersion);
			Epoch::Utils::HashCombine(result, aMetadata.revisionVersion);
			return result;
		}
	};

}
