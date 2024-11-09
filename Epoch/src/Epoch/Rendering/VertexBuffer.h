#pragma once
#include <string>
#include <vector>
#include "Epoch/Debug/Log.h"
#include <Epoch/Core/Buffer.h>

namespace Epoch
{
	enum class ShaderDataType { Float, Float2, Float3, Float4, Int, Int2, Int3, Int4, UInt, UInt2, UInt3, UInt4 };
	enum class ShaderDataInputRate { PerVertex, PerInstance };

	static unsigned ShaderDataTypeSize(ShaderDataType aType)
	{
		switch (aType)
		{
			case ShaderDataType::Float:		return 4;
			case ShaderDataType::Float2:	return 4 * 2;
			case ShaderDataType::Float3:	return 4 * 3;
			case ShaderDataType::Float4:	return 4 * 4;
			case ShaderDataType::Int:		return 4;
			case ShaderDataType::Int2:		return 4 * 2;
			case ShaderDataType::Int3:		return 4 * 3;
			case ShaderDataType::Int4:		return 4 * 4;
			case ShaderDataType::UInt:		return 4;
			case ShaderDataType::UInt2:		return 4 * 2;
			case ShaderDataType::UInt3:		return 4 * 3;
			case ShaderDataType::UInt4:		return 4 * 4;
		}

		EPOCH_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	struct VertexBufferElement
	{
		std::string name;
		ShaderDataType type;
		uint32_t size;
		uint32_t offset;
		uint32_t index;

		VertexBufferElement() = delete;
		VertexBufferElement(ShaderDataType aType, const std::string& aName, uint32_t aIndex = 0) :
			name(aName), type(aType), size(ShaderDataTypeSize(aType)), offset(0), index(aIndex) {}
	};

	class VertexBufferLayout
	{
	public:
		VertexBufferLayout() = default;
		VertexBufferLayout(const std::initializer_list<VertexBufferElement>& aElements) : myElements(aElements) { CalculateOffsetsAndStride(); }
		VertexBufferLayout(ShaderDataInputRate aInpuRate, const std::initializer_list<VertexBufferElement>& aElements) : myInpuRate(aInpuRate), myElements(aElements) { CalculateOffsetsAndStride(); }

		uint32_t GetStride() const { return myStride; }
		ShaderDataInputRate GetInpuRate() const { return myInpuRate; }
		bool Emtpy() const { return myElements.empty(); }
		uint32_t GetElementCount() const { return (uint32_t)myElements.size(); }
		
		[[nodiscard]] std::vector<VertexBufferElement>::iterator begin() { return myElements.begin(); }
		[[nodiscard]] std::vector<VertexBufferElement>::iterator end() { return myElements.end(); }
		[[nodiscard]] std::vector<VertexBufferElement>::const_iterator begin() const { return myElements.begin(); }
		[[nodiscard]] std::vector<VertexBufferElement>::const_iterator end() const { return myElements.end(); }

	private:
		void CalculateOffsetsAndStride()
		{
			unsigned offset = 0;
			myStride = 0;
			for (auto& element : myElements)
			{
				element.offset = offset;
				offset += element.size;
				myStride += element.size;
			}
		}

	private:
		std::vector<VertexBufferElement> myElements;
		uint32_t myStride = 0;
		ShaderDataInputRate myInpuRate = ShaderDataInputRate::PerVertex;
	};

	enum class VertexBufferUsage { None, Static, Dynamic };

	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() = default;
		
		static std::shared_ptr<VertexBuffer> Create(void* aData, uint32_t aCount, uint32_t aStride, VertexBufferUsage aUsage = VertexBufferUsage::Static);
		static std::shared_ptr<VertexBuffer> Create(uint32_t aCount, uint32_t aStride);

		virtual void SetData(void* aBuffer, uint32_t aCount, uint64_t aOffset = 0) = 0;
		virtual void Resize(uint32_t aCount) = 0;

		VertexBufferUsage GetUsage() const { return myUsage; }
		uint32_t GetCount() const { return myCount; }
		uint32_t GetStride() const { return myStride; }
		uint64_t GetSize() const { return mySize; }

	protected:
		VertexBufferUsage myUsage = VertexBufferUsage::None;
		Buffer myLocalData;
		uint32_t myCount = 0;
		uint32_t myStride = 0;
		uint64_t mySize = 0;
	};
}
