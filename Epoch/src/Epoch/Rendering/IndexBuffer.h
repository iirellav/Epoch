#pragma once
#include <memory>
#include <Epoch/Core/Buffer.h>

namespace Epoch
{
	enum class IndexBufferUsage { None, Static, Dynamic };

	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;
		
		static std::shared_ptr<IndexBuffer> Create(void* aData, uint32_t aCount, IndexBufferUsage aUsage = IndexBufferUsage::Static);
		static std::shared_ptr<IndexBuffer> Create(uint32_t aCount);

		virtual void SetData(void* aBuffer, uint32_t aCount, uint64_t aOffset = 0) = 0;
		virtual void Resize(uint32_t aCount) = 0;

		IndexBufferUsage GetUsage() const { return myUsage; }
		uint64_t GetSize() const { return mySize; }
		uint32_t GetCount() { return myCount; }

	protected:
		IndexBufferUsage myUsage = IndexBufferUsage::None;
		Buffer myLocalData;
		uint32_t myCount = 0;
		uint64_t mySize = 0;
	};
}
