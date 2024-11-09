#pragma once
#include <CommonUtilities/Math/Vector/Vector.h>
#include "Epoch/Core/Buffer.h"
#include "Epoch/Core/UUID.h"
#include "GCManager.h"
#include "ScriptTypes.h"

namespace Epoch
{
	enum class FieldFlag
	{
		None = -1,
		ReadOnly =	1u << 0,
		Static =	1u << 1,
		Public =	1u << 2,
		Private =	1u << 3,
		Protected = 1u << 4,
		Internal =	1u << 5,
		IsArray =	1u << 6
	};

	enum class FieldType
	{
		Void,
		Bool,
		Int8, Int16, Int32, Int64,
		UInt8, UInt16, UInt32, UInt64,
		Float, Double,
		String,
		Vector2, Vector3,
		Color,
		AssetHandle,
		Scene,
		Prefab,
		Entity,
		Material,
		Mesh
	};

	struct FieldInfo
	{
		uint32_t id;
		std::string name;
		FieldType type;
		uint32_t size;
		bool isProperty;
		Buffer defaultValueBuffer;

		uint64_t flags = 0;

		//Editor Only
		std::string tooltip;
		std::string header;
		uint32_t spacing;


		bool HasFlag(FieldFlag aFlag) const { return flags & (uint64_t)aFlag; }

		bool IsWritable() const
		{
			return !HasFlag(FieldFlag::ReadOnly) && HasFlag(FieldFlag::Public);
		}

		bool IsArray() const { return HasFlag(FieldFlag::IsArray); }
	};
	
	namespace FieldUtils
	{
		inline FieldType StringToFieldType(const std::string& aTypeString)
		{
			if (aTypeString == "Void")			return FieldType::Void;
			if (aTypeString == "Bool")			return FieldType::Bool;
			if (aTypeString == "Int8")			return FieldType::Int8;
			if (aTypeString == "Int16")			return FieldType::Int16;
			if (aTypeString == "Int32")			return FieldType::Int32;
			if (aTypeString == "Int64")			return FieldType::Int64;
			if (aTypeString == "UInt8")			return FieldType::UInt8;
			if (aTypeString == "UInt16")		return FieldType::UInt16;
			if (aTypeString == "UInt32")		return FieldType::UInt32;
			if (aTypeString == "UInt64")		return FieldType::UInt64;
			if (aTypeString == "Float")			return FieldType::Float;
			if (aTypeString == "Double")		return FieldType::Double;
			if (aTypeString == "String")		return FieldType::String;
			if (aTypeString == "Vector2")		return FieldType::Vector2;
			if (aTypeString == "Vector3")		return FieldType::Vector3;
			if (aTypeString == "Color")			return FieldType::Color;
			if (aTypeString == "AssetHandle")	return FieldType::AssetHandle;
			if (aTypeString == "Scene")			return FieldType::Scene;
			if (aTypeString == "Prefab")		return FieldType::Prefab;
			if (aTypeString == "Entity")		return FieldType::Entity;
			if (aTypeString == "Material")		return FieldType::Material;
			if (aTypeString == "Mesh")			return FieldType::Mesh;

			EPOCH_ASSERT(false, "Unknown script field type!");
			return FieldType::Void;
		}

		inline const char* FieldTypeToString(FieldType aFieldType)
		{
			switch (aFieldType)
			{
				case FieldType::Void:			return "Void";
				case FieldType::Bool:			return "Bool";
				case FieldType::Int8:			return "Int8";
				case FieldType::Int16:			return "Int16";
				case FieldType::Int32:			return "Int32";
				case FieldType::Int64:			return "Int64";
				case FieldType::UInt8:			return "UInt8";
				case FieldType::UInt16:			return "UInt16";
				case FieldType::UInt32:			return "UInt32";
				case FieldType::UInt64:			return "UInt64";
				case FieldType::Float:			return "Float";
				case FieldType::Double:			return "Double";
				case FieldType::String:			return "String";
				case FieldType::Vector2:		return "Vector2";
				case FieldType::Vector3:		return "Vector3";
				case FieldType::Color:			return "Color";
				case FieldType::AssetHandle:	return "AssetHandle";
				case FieldType::Scene:			return "Scene";
				case FieldType::Prefab:			return "Prefab";
				case FieldType::Entity:			return "Entity";
				case FieldType::Material:		return "Material";
				case FieldType::Mesh:			return "Mesh";
			}
			
			EPOCH_ASSERT(false, "Unknown script field type!");
			return "Unknown";
		}

		inline uint32_t GetFieldTypeSize(FieldType aType)
		{
			switch (aType)
			{
				case FieldType::Bool:		return sizeof(bool);
				case FieldType::Int8:		return sizeof(int8_t);
				case FieldType::Int16:		return sizeof(int16_t);
				case FieldType::Int32:		return sizeof(int32_t);
				case FieldType::Int64:		return sizeof(int64_t);
				case FieldType::UInt8:		return sizeof(uint8_t);
				case FieldType::UInt16:		return sizeof(uint16_t);
				case FieldType::UInt32:		return sizeof(uint32_t);
				case FieldType::UInt64:		return sizeof(uint64_t);
				case FieldType::Float:		return sizeof(float);
				case FieldType::Double:		return sizeof(double);
				case FieldType::String:		return sizeof(char);
				case FieldType::Vector2:	return sizeof(CU::Vector2f);
				case FieldType::Vector3:	return sizeof(CU::Vector3f);
				case FieldType::Color:		return sizeof(CU::Color);
				case FieldType::AssetHandle:
				case FieldType::Scene:
				case FieldType::Entity:
				case FieldType::Prefab:
				case FieldType::Material:
				case FieldType::Mesh:
				return sizeof(UUID);
			}

			EPOCH_ASSERT(false, "Unknown script field type!");
			return 0;
		}
		
		inline bool IsPrimitiveType(FieldType aType)
		{
			switch (aType)
			{
				case FieldType::String: return false;
				case FieldType::Scene:	return false;
				case FieldType::Entity: return false;
				case FieldType::Prefab: return false;
				case FieldType::Material: return false;
				case FieldType::Mesh: return false;
			}

			return true;
		}

		inline bool IsAsset(FieldType type)
		{
			switch (type)
			{
				case FieldType::Prefab: return true;
				case FieldType::Scene: return true;
				case FieldType::Material: return true;
				case FieldType::Mesh: return true;
			}

			return false;
		}
	}

	class FieldStorageBase
	{
	public:
		FieldStorageBase(FieldInfo* aFieldInfo) : myFieldInfo(aFieldInfo) {}

		virtual void SetRuntimeInstance(GCHandle aInstance) = 0;
		virtual void CopyFrom(const std::shared_ptr<FieldStorageBase>& aOther) = 0;

		virtual Buffer GetValueBuffer() const = 0;
		virtual void SetValueBuffer(const Buffer& aBuffer) = 0;

		const FieldInfo* GetFieldInfo() const { return myFieldInfo; }

	protected:
		FieldInfo* myFieldInfo = nullptr;
	};

	class FieldStorage : public FieldStorageBase
	{
	public:
		FieldStorage(FieldInfo* aFieldInfo) : FieldStorageBase(aFieldInfo)
		{
			myDataBuffer = Buffer::Copy(aFieldInfo->defaultValueBuffer);
		}

		template<typename T>
		T GetValue() const
		{
			if (myRuntimeInstance != nullptr)
			{
				Buffer valueBuffer;
				bool success = GetValueRuntime(valueBuffer);

				if (!success)
				{
					return T();
				}

				T value = T();
				memcpy(&value, valueBuffer.data, valueBuffer.size);
				valueBuffer.Release();
				return value;
			}

			if (!myDataBuffer)
			{
				return T();
			}

			return *(myDataBuffer.As<T>());
		}

		template<>
		std::string GetValue() const
		{
			if (myRuntimeInstance != nullptr)
			{
				Buffer valueBuffer;
				bool success = GetValueRuntime(valueBuffer);

				if (!success)
				{
					return std::string();
				}

				std::string value((char*)valueBuffer.data, valueBuffer.size / sizeof(char));
				valueBuffer.Release();
				return value;
			}

			if (!myDataBuffer)
			{
				return std::string();
			}

			return std::string((char*)myDataBuffer.data);
		}

		template<typename T>
		void SetValue(const T& aValue)
		{
			EPOCH_ASSERT(sizeof(T) == myFieldInfo->size, "Not same size!");

			if (myRuntimeInstance != nullptr)
			{
				SetValueRuntime(&aValue);
			}
			else
			{
				if (!myDataBuffer)
				{
					myDataBuffer.Allocate(myFieldInfo->size);
				}
				myDataBuffer.Write(&aValue, sizeof(T));
			}
		}

		template<>
		void SetValue<std::string>(const std::string& aValue)
		{
			if (myRuntimeInstance != nullptr)
			{
				SetValueRuntime(&aValue);
			}
			else
			{
				if (myDataBuffer.size <= aValue.length() * sizeof(char))
				{
					myDataBuffer.Release();
					myDataBuffer.Allocate((aValue.length() * 2) * sizeof(char));
				}
				
				myDataBuffer.ZeroInitialize();
				strcpy((char*)myDataBuffer.data, aValue.c_str());
			}
		}

		virtual void SetRuntimeInstance(GCHandle aInstance) override
		{
			myRuntimeInstance = aInstance;

			if (myRuntimeInstance)
			{
				if (myFieldInfo->type == FieldType::String)
				{
					std::string str((char*)myDataBuffer.data, myDataBuffer.size / sizeof(char));
					SetValueRuntime(&str);
				}
				else
				{
					SetValueRuntime(myDataBuffer.data);
				}
			}
		}

		virtual void CopyFrom(const std::shared_ptr<FieldStorageBase>& aOther)
		{
			std::shared_ptr<FieldStorage> fieldStorage = std::dynamic_pointer_cast<FieldStorage>(aOther);

			if (myRuntimeInstance != nullptr)
			{
				Buffer valueBuffer;
				if (fieldStorage->GetValueRuntime(valueBuffer))
				{
					SetValueRuntime(valueBuffer.data);
					valueBuffer.Release();
				}
			}
			else
			{
				myDataBuffer.Release();
				myDataBuffer = Buffer::Copy(fieldStorage->myDataBuffer);
			}
		}

		virtual Buffer GetValueBuffer() const override
		{
			if (myRuntimeInstance == nullptr)
			{
				return myDataBuffer;
			}

			Buffer result;
			GetValueRuntime(result);
			return result;
		}

		virtual void SetValueBuffer(const Buffer& aBuffer)
		{
			if (myRuntimeInstance != nullptr)
			{
				SetValueRuntime(aBuffer.data);
			}
			else
			{
				myDataBuffer = Buffer::Copy(aBuffer);
			}
		}

	private:
		bool GetValueRuntime(Buffer& outBuffer) const;
		void SetValueRuntime(const void* aData);

	private:
		Buffer myDataBuffer;
		GCHandle myRuntimeInstance = nullptr;
	};

	class ArrayFieldStorage : public FieldStorageBase
	{
	public:
		ArrayFieldStorage(FieldInfo* fieldInfo) : FieldStorageBase(fieldInfo)
		{
			myDataBuffer = Buffer::Copy(fieldInfo->defaultValueBuffer);
			myLength = (uint32_t)(myDataBuffer.size / myFieldInfo->size);
		}

		template<typename T>
		T GetValue(uint32_t aIndex) const
		{
			if (myRuntimeInstance != nullptr)
			{
				T value = T();
				GetValueRuntime(aIndex, &value);
				return value;
			}

			if (!myDataBuffer)
			{
				return T();
			}

			uint32_t offset = aIndex * sizeof(T);
			return myDataBuffer.Read<T>(offset);
		}

		template<>
		std::string GetValue(uint32_t aIndex) const
		{
			return "";
		}

		template<typename T>
		void SetValue(uint32_t aIndex, const T& aValue)
		{
			EPOCH_ASSERT(sizeof(T) == myFieldInfo->size);

			if (myRuntimeInstance != nullptr)
			{
				SetValueRuntime(aIndex, &aValue);
			}
			else
			{
				myDataBuffer.Write(&aValue, sizeof(T), aIndex * sizeof(T));
			}
		}

		template<>
		void SetValue<std::string>(uint32_t aIndex, const std::string& aValue)
		{
			if (myRuntimeInstance != nullptr)
			{
				SetValueRuntime(aIndex, &aValue);
			}
			else
			{
				auto& stringBuffer = (Buffer&)myDataBuffer[aIndex * sizeof(Buffer)];
				if (stringBuffer.size != aValue.size())
				{
					stringBuffer.Release();
					stringBuffer.Allocate((aValue.length() * 2) * sizeof(char));
				}

				stringBuffer.ZeroInitialize();
				memcpy(stringBuffer.data, aValue.c_str(), aValue.length() * sizeof(char));
			}
		}

		virtual void SetRuntimeInstance(GCHandle aInstance) override
		{
			myRuntimeInstance = aInstance;

			if (myFieldInfo->type == FieldType::String)
				return;

			if (myRuntimeInstance)
			{
				SetRuntimeArray(myDataBuffer);
			}
		}

		virtual void CopyFrom(const std::shared_ptr<FieldStorageBase>& aOther)
		{
			std::shared_ptr<ArrayFieldStorage> fieldStorage = std::dynamic_pointer_cast<ArrayFieldStorage>(aOther);

			if (myFieldInfo->type == FieldType::String)
			{
				return;
			}

			if (myRuntimeInstance != nullptr)
			{
				Buffer valueBuffer;
				if (fieldStorage->GetRuntimeArray(valueBuffer))
				{
					SetRuntimeArray(valueBuffer);
					valueBuffer.Release();
				}
			}
			else
			{
				myDataBuffer.Release();
				myDataBuffer = Buffer::Copy(fieldStorage->myDataBuffer);
			}
		}

		void Resize(uint32_t aNewLength);
		void RemoveAt(uint32_t aIndex);

		uint32_t GetLength() const { return myRuntimeInstance != nullptr ? GetLengthRuntime() : myLength; }

		virtual Buffer GetValueBuffer() const override
		{
			if (myFieldInfo->type == FieldType::String)
			{
				return Buffer();
			}

			if (myRuntimeInstance == nullptr)
			{
				return myDataBuffer;
			}

			Buffer result;
			GetRuntimeArray(result);
			return result;
		}

		virtual void SetValueBuffer(const Buffer& aBuffer)
		{
			if (myFieldInfo->type == FieldType::String)
			{
				return;
			}

			if (myRuntimeInstance != nullptr)
			{
				SetRuntimeArray(aBuffer);
				myLength = (uint32_t)(aBuffer.size / myFieldInfo->size);
			}
			else
			{
				myDataBuffer = Buffer::Copy(myFieldInfo->defaultValueBuffer);

				if (myDataBuffer.size < aBuffer.size)
				{
					myDataBuffer.Release();
					myDataBuffer = Buffer::Copy(aBuffer);
				}
				else
				{
					myDataBuffer.Write(aBuffer.data, aBuffer.size);
				}

				myLength = (uint32_t)(myDataBuffer.size / myFieldInfo->size);
			}
		}
		
	private:
		bool GetRuntimeArray(Buffer& outData) const;
		void SetRuntimeArray(const Buffer& aData);
		void GetValueRuntime(uint32_t aIndex, void* aData) const;
		void SetValueRuntime(uint32_t aIndex, const void* aData);
		uint32_t GetLengthRuntime() const;

	private:
		Buffer myDataBuffer;
		uint32_t myLength = 0;
		GCHandle myRuntimeInstance = nullptr;
	};
}
