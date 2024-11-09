#pragma once
#include <CommonUtilities/Math/Vector/Vector3.hpp>
#include "Epoch/Core/Buffer.h"

namespace Epoch::Utils
{
	//class ValueWrapper
	//{
	//public:
	//	ValueWrapper() = default;
	//	ValueWrapper(bool aValue) : ValueWrapper(&aValue, sizeof(bool)) {}
	//	ValueWrapper(int8_t aValue) : ValueWrapper(&aValue, sizeof(int8_t)) {}
	//	ValueWrapper(int16_t aValue) : ValueWrapper(&aValue, sizeof(int16_t)) {}
	//	ValueWrapper(int32_t aValue) : ValueWrapper(&aValue, sizeof(int32_t)) {}
	//	ValueWrapper(int64_t aValue) : ValueWrapper(&aValue, sizeof(int64_t)) {}
	//	ValueWrapper(uint8_t aValue) : ValueWrapper(&aValue, sizeof(uint8_t)) {}
	//	ValueWrapper(uint16_t aValue) : ValueWrapper(&aValue, sizeof(uint16_t)) {}
	//	ValueWrapper(uint32_t aValue) : ValueWrapper(&aValue, sizeof(uint32_t)) {}
	//	ValueWrapper(uint64_t aValue) : ValueWrapper(&aValue, sizeof(uint64_t)) {}
	//	ValueWrapper(float aValue) : ValueWrapper(&aValue, sizeof(float)) {}
	//	ValueWrapper(double aValue) : ValueWrapper(&aValue, sizeof(double)) {}
	//	ValueWrapper(const std::string& aValue) : ValueWrapper(static_cast<const void*>(aValue.c_str()), aValue.size()) {}
	//	ValueWrapper(const CU::Vector3f &aValue) : ValueWrapper(static_cast<const void*>(&aValue.x), sizeof(float) * 3) {}
	//
	//	ValueWrapper(const void* aData, size_t aSize) : mySize(aSize)
	//	{
	//		myValueBuffer = new byte[aSize];
	//		memcpy(myValueBuffer, aData, aSize);
	//	}
	//
	//	ValueWrapper(const ValueWrapper& aOther)
	//	{
	//		mySize = aOther.mySize;
	//		if (mySize)
	//		{
	//			myValueBuffer = new byte[mySize];
	//			memcpy(myValueBuffer, aOther.myValueBuffer, mySize);
	//		}
	//	}
	//
	//	ValueWrapper& operator=(const ValueWrapper& aOther)
	//	{
	//		if (myValueBuffer != aOther.myValueBuffer)
	//		{
	//			ReleaseBuffer();
	//
	//			mySize = aOther.mySize;
	//			if (mySize)
	//			{
	//				myValueBuffer = new byte[mySize];
	//				memcpy(myValueBuffer, aOther.myValueBuffer, mySize);
	//			}
	//			else
	//			{
	//				myValueBuffer = nullptr;
	//			}
	//		}
	//
	//		return *this;
	//	}
	//
	//	~ValueWrapper()
	//	{
	//		mySize = 0;
	//		delete[] myValueBuffer;
	//		myValueBuffer = nullptr;
	//	}
	//
	//	void ReleaseBuffer()
	//	{
	//		mySize = 0;
	//		delete[] myValueBuffer;
	//		myValueBuffer = nullptr;
	//	}
	//
	//	bool HasValue() const { return mySize > 0 && myValueBuffer != nullptr; }
	//
	//	template<typename TValueType>
	//	TValueType Get() const
	//	{
	//		EPOCH_ASSERT(HasValue(), "ValueWrapper::Get - No value!");
	//
	//		if constexpr (std::is_same<TValueType, std::string>::value)
	//		{
	//			return TValueType((char*)myValueBuffer, mySize);
	//		}
	//		else
	//		{
	//			return *(TValueType*)myValueBuffer;
	//		}
	//	}
	//
	//	template<typename TValueType>
	//	TValueType GetOrDefault(const TValueType& aDefaultValue) const
	//	{
	//		if (!HasValue())
	//		{
	//			return aDefaultValue;
	//		}
	//
	//		if constexpr (std::is_same<TValueType, std::string>::value)
	//		{
	//			return TValueType((char*)myValueBuffer, mySize);
	//		}
	//		else
	//		{
	//			return *(TValueType*)myValueBuffer;
	//		}
	//	}
	//
	//	void* GetRawData() const { return myValueBuffer; }
	//	size_t GetDataSize() const { return mySize; }
	//
	//	template<typename TValueType>
	//	void Set(TValueType aValue)
	//	{
	//		EPOCH_ASSERT(HasValue(), "Trying to set the value of an empty ValueWrapper!");
	//
	//		if constexpr (std::is_same<TValueType, std::string>::value)
	//		{
	//			if (aValue.size() > mySize)
	//			{
	//				delete[] myValueBuffer;
	//				myValueBuffer = new byte[aValue.size()];
	//				mySize = aValue.size();
	//			}
	//
	//			memcpy(myValueBuffer, aValue.data(), aValue.size());
	//		}
	//		else
	//		{
	//			EPOCH_ASSERT(mySize == sizeof(aValue), "Wrong size!");
	//			memcpy(myValueBuffer, &aValue, sizeof(aValue));
	//		}
	//	}
	//
	//	static ValueWrapper OfSize(size_t aSize)
	//	{
	//		ValueWrapper value;
	//		value.myValueBuffer = new byte[aSize];
	//		value.mySize = aSize;
	//		return value;
	//	}
	//
	//	static ValueWrapper Empty() { return ValueWrapper(); }
	//
	//private:
	//	size_t mySize = 0;
	//	byte* myValueBuffer = nullptr;
	//};
}
