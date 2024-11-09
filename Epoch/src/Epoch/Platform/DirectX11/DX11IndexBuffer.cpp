#include "epch.h"
#include "DX11IndexBuffer.h"
#include "Epoch/Rendering/RHI.h"

namespace Epoch
{
	DX11IndexBuffer::DX11IndexBuffer(void* aData, uint32_t aCount, IndexBufferUsage aUsage)
	{
		EPOCH_PROFILE_FUNC();

		myUsage = aUsage;
		myCount = aCount;
		mySize = aCount * sizeof(uint32_t);
		myLocalData = Buffer::Copy(aData, mySize);

		D3D11_BUFFER_DESC indexBufferDescription{};
		indexBufferDescription.ByteWidth = (UINT)mySize;
		indexBufferDescription.Usage = aUsage == IndexBufferUsage::Static ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DYNAMIC;
		indexBufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDescription.CPUAccessFlags = aUsage == IndexBufferUsage::Static ? 0 : D3D10_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA indexSubResourceData{};
		indexSubResourceData.pSysMem = aData;

		const HRESULT result = RHI::GetDevice()->CreateBuffer(&indexBufferDescription, &indexSubResourceData, myBuffer.GetAddressOf());
		if (FAILED(result))
		{
			EPOCH_ASSERT(false, "Failed to create index buffer!");
		}
	}

	DX11IndexBuffer::DX11IndexBuffer(uint32_t aCount)
	{
		EPOCH_PROFILE_FUNC();

		myUsage = IndexBufferUsage::Dynamic;
		myCount = aCount;
		mySize = aCount * sizeof(uint32_t);
		myLocalData.Allocate(mySize);

		D3D11_BUFFER_DESC indexBufferDescription{};
		indexBufferDescription.ByteWidth = (UINT)mySize;
		indexBufferDescription.Usage = D3D11_USAGE_DYNAMIC;
		indexBufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDescription.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		
		const HRESULT result = RHI::GetDevice()->CreateBuffer(&indexBufferDescription, nullptr, myBuffer.GetAddressOf());
		if (FAILED(result))
		{
			EPOCH_ASSERT(false, "Failed to create index buffer!");
		}
	}

	DX11IndexBuffer::~DX11IndexBuffer()
	{
		//myBuffer->Release();
		myLocalData.Release();
	}

	void DX11IndexBuffer::SetData(void* aBuffer, uint32_t aCount, uint64_t aOffset)
	{
		EPOCH_PROFILE_FUNC();

		uint64_t size = aCount * sizeof(uint32_t);
		EPOCH_ASSERT(size <= mySize, "Buffer overflow!");
		EPOCH_ASSERT(myUsage == IndexBufferUsage::Dynamic, "Tried to set the data of a static buffer!");
		
		D3D11_MAPPED_SUBRESOURCE bufferData{};
		ZeroMemory(&bufferData, sizeof(D3D11_MAPPED_SUBRESOURCE));

		HRESULT result = RHI::GetContext()->Map(myBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);
		if (FAILED(result))
		{
			EPOCH_ASSERT(false, "Failed to map resource!");
		}

		memcpy(bufferData.pData, (uint8_t*)aBuffer + aOffset, size);

		RHI::GetContext()->Unmap(myBuffer.Get(), 0);
		
		myCount = aCount;
		memcpy(myLocalData.data, (uint8_t*)aBuffer + aOffset, size);
	}

	void DX11IndexBuffer::Resize(uint32_t aCount)
	{
		EPOCH_ASSERT(myUsage == IndexBufferUsage::Dynamic, "Tried to resize a static buffer!");

		myUsage = IndexBufferUsage::Dynamic;
		myCount = aCount;
		mySize = aCount * sizeof(uint32_t);
		myLocalData.Allocate(mySize);

		D3D11_BUFFER_DESC indexBufferDesc{};
		indexBufferDesc.ByteWidth = (UINT)mySize;
		indexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		
		const HRESULT result = RHI::GetDevice()->CreateBuffer(&indexBufferDesc, nullptr, myBuffer.ReleaseAndGetAddressOf());
		if (FAILED(result))
		{
			EPOCH_ASSERT(false, "Failed to resize index buffer!");
		}
	}
}
