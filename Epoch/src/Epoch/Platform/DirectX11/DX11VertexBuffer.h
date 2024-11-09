#pragma once
#include "Epoch/Rendering/VertexBuffer.h"
#include <d3d11.h>
#include <wrl.h>

using namespace Microsoft::WRL;

namespace Epoch
{
	class DX11VertexBuffer : public VertexBuffer
	{
	public:
		DX11VertexBuffer(void* aData, uint32_t aCount, uint32_t aStride, VertexBufferUsage aUsage = VertexBufferUsage::Static);
		DX11VertexBuffer(uint32_t aCount, uint32_t aStride);
		~DX11VertexBuffer() override;

		void SetData(void* aBuffer, uint32_t aCount, uint64_t aOffset = 0) override;
		void Resize(uint32_t aCount) override;

		ComPtr<ID3D11Buffer> GetDXBuffer() const { return myBuffer; }

	private:
		ComPtr<ID3D11Buffer> myBuffer;
	};
}
