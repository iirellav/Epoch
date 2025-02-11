#pragma once
#include "Epoch/Rendering/ConstantBuffer.h"
#include <d3d11.h>
#include <wrl.h>

using namespace Microsoft::WRL;

namespace Epoch
{
	class DX11ConstantBuffer : public ConstantBuffer
	{
	public:
		DX11ConstantBuffer(void* aData, uint64_t aSize);
		DX11ConstantBuffer(uint64_t aSize);
		~DX11ConstantBuffer() override;

		void SetData(void* aBuffer, uint64_t aSize = 0, uint64_t aOffset = 0) override;

		void Bind(ShaderStage aStages, unsigned aSlot) override;

	private:
		ComPtr<ID3D11Buffer> myBuffer;
	};
}
