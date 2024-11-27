#include "epch.h"
#include "DX11ConstantBuffer.h"
#include "Epoch/Rendering/RHI.h"

namespace Epoch
{
	DX11ConstantBuffer::DX11ConstantBuffer(void* aData, uint64_t aSize)
	{
		EPOCH_PROFILE_FUNC();

		EPOCH_ASSERT(aSize <= 65536, "Exceeded max CBuffer size per buffer in bytes!");
		
		if (CU::Math::Mod((float)aSize / 16.0f) > 0.0f)
		{
			EPOCH_ASSERT(false, "Content buffer must be a multiple of 16! size = " + std::to_string(aSize));
		}

		myLocalData = Buffer::Copy(aData, aSize);
		mySize = aSize;

		D3D11_BUFFER_DESC bufferDescription{};
		bufferDescription.Usage = D3D11_USAGE_DYNAMIC;
		bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDescription.ByteWidth = (unsigned)aSize;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData{};
		vertexSubResourceData.pSysMem = aData;

		const HRESULT result = RHI::GetDevice()->CreateBuffer(&bufferDescription, &vertexSubResourceData, myBuffer.GetAddressOf());
		if (FAILED(result))
		{
			EPOCH_ASSERT(false, "Failed to create constant buffer!");
		}
	}

	DX11ConstantBuffer::DX11ConstantBuffer(uint64_t aSize)
	{
		EPOCH_PROFILE_FUNC();

		EPOCH_ASSERT(aSize <= 65536, "Exceeded max CBuffer size per buffer in bytes!");
		
		if (CU::Math::Mod((float)aSize / 16.0f) > 0.0f)
		{
			EPOCH_ASSERT(false, "Content buffer must be a multiple of 16! size = " + std::to_string(aSize));
		}

		mySize = aSize;
		myLocalData.Allocate(mySize);

		D3D11_BUFFER_DESC bufferDescription{};
		bufferDescription.Usage = D3D11_USAGE_DYNAMIC;
		bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDescription.ByteWidth = (unsigned)aSize;

		const HRESULT result = RHI::GetDevice()->CreateBuffer(&bufferDescription, nullptr, myBuffer.GetAddressOf());
		if (FAILED(result))
		{
			EPOCH_ASSERT(false, "Failed to create constant buffer!");
		}
	}

	DX11ConstantBuffer::~DX11ConstantBuffer()
	{
		myLocalData.Release();
	}

	void DX11ConstantBuffer::SetData(void* aBuffer, uint64_t aSize, uint64_t aOffset)
	{
		EPOCH_ASSERT(aSize <= mySize, "Buffer overflow!");
		
		if (aSize == 0) aSize = mySize;

		D3D11_MAPPED_SUBRESOURCE bufferData;
		ZeroMemory(&bufferData, sizeof(D3D11_MAPPED_SUBRESOURCE));

		HRESULT result = RHI::GetContext()->Map(myBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);
		if (FAILED(result))
		{
			EPOCH_ASSERT(false, "Failed to map resource!");
		}

		memcpy(bufferData.pData, (uint8_t*)aBuffer + aOffset, aSize);

		RHI::GetContext()->Unmap(myBuffer.Get(), 0);

		memcpy(myLocalData.data, (uint8_t*)aBuffer + aOffset, aSize);
	}

	void DX11ConstantBuffer::Bind(UINT aPipelineStages, unsigned aSlot)
	{
		if (aPipelineStages & PIPELINE_STAGE_VERTEX_SHADER)
		{
			RHI::GetContext()->VSSetConstantBuffers(aSlot, 1, myBuffer.GetAddressOf());
		}

		if (aPipelineStages & PIPELINE_STAGE_PIXEL_SHADER)
		{
			RHI::GetContext()->PSSetConstantBuffers(aSlot, 1, myBuffer.GetAddressOf());
		}

		if (aPipelineStages & PIPELINE_STAGE_COMPUTE_SHADER)
		{
			RHI::GetContext()->CSSetConstantBuffers(aSlot, 1, myBuffer.GetAddressOf());
		}
	}
}
