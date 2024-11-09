#pragma once
#include "Epoch/Rendering/IndexBuffer.h"
#include <d3d11.h>
#include <wrl.h>

using namespace Microsoft::WRL;

namespace Epoch
{
	class DX11IndexBuffer : public IndexBuffer
	{
	public:
		DX11IndexBuffer(void* aData, uint32_t aCount, IndexBufferUsage aUsage = IndexBufferUsage::Static);
		DX11IndexBuffer(uint32_t aCount);
		~DX11IndexBuffer() override;

		void SetData(void* aBuffer, uint32_t aCount, uint64_t aOffset = 0) override;
		void Resize(uint32_t aCount) override;
		
		ComPtr<ID3D11Buffer> GetDXBuffer() const { return myBuffer; }

	private:
		ComPtr<ID3D11Buffer> myBuffer;
	};
}
