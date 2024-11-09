#pragma once
#include "Epoch/Core/Buffer.h"

extern "C"
{
	typedef struct _MonoObject MonoObject;
}

namespace Epoch
{
	class CSharpInstanceInspector
	{
	public:
		CSharpInstanceInspector(MonoObject* aInstance);

		bool HasName(std::string_view aFullName) const;
		bool InheritsFrom(std::string_view aFullName) const;

		const char* GetName() const;

		template<typename T>
		T GetFieldValue(std::string_view aFieldName) const
		{
			Buffer valueBuffer = GetFieldBuffer(aFieldName);

			if (!valueBuffer)
			{
				return T();
			}

			T value = T();
			memcpy(&value, valueBuffer.data, sizeof(T));
			valueBuffer.Release();
			return value;
		}

		template<>
		std::string GetFieldValue(std::string_view aFieldName) const
		{
			Buffer valueBuffer = GetFieldBuffer(aFieldName);

			if (!valueBuffer)
			{
				return std::string();
			}

			std::string value((char*)valueBuffer.data, valueBuffer.size / sizeof(char));
			valueBuffer.Release();
			return value;
		}

	private:
		Buffer GetFieldBuffer(std::string_view aFieldName) const;

	private:
		MonoObject* myInstance;
	};
}
