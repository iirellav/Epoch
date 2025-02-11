#pragma once
#include <memory>
#include <Epoch/Core/Buffer.h>
#include "Epoch/Rendering/Shader.h"

namespace Epoch
{
	class ConstantBuffer
	{
	public:
		virtual ~ConstantBuffer() = default;

		static std::shared_ptr<ConstantBuffer> Create(void* aData, uint64_t aSize);
		static std::shared_ptr<ConstantBuffer> Create(uint64_t aSize);
		
		virtual void SetData(void* aBuffer, uint64_t aSize = 0, uint64_t aOffset = 0) = 0;
		uint64_t GetSize() const { return mySize; }
		
		virtual void Bind(ShaderStage aStages, unsigned aSlot) = 0;

	protected:
		Buffer myLocalData;
		uint64_t mySize = 0;
	};
}
