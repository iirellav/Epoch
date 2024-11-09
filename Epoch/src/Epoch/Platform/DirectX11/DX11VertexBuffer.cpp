#include "epch.h"
#include "DX11VertexBuffer.h"
#include "Epoch/Rendering/RHI.h"

namespace Epoch
{
	DX11VertexBuffer::DX11VertexBuffer(void* aData, uint32_t aCount, uint32_t aStride, VertexBufferUsage aUsage)
	{
		EPOCH_PROFILE_FUNC();

		myUsage = aUsage;
		myCount = aCount;
		myStride = aStride;
		mySize = aCount * aStride;
		myLocalData = Buffer::Copy(aData, mySize);

		D3D11_BUFFER_DESC vertexBufferDescription{};
		vertexBufferDescription.ByteWidth = (UINT)mySize;
		vertexBufferDescription.Usage = aUsage == VertexBufferUsage::Static ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DYNAMIC;
		vertexBufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDescription.CPUAccessFlags = aUsage == VertexBufferUsage::Static ? 0 : D3D10_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData{};
		vertexSubResourceData.pSysMem = aData;

		const HRESULT result = RHI::GetDevice()->CreateBuffer(&vertexBufferDescription, &vertexSubResourceData, myBuffer.GetAddressOf());
		if (FAILED(result))
		{
			EPOCH_ASSERT(false, "Failed to create vertex buffer!");
		}
	}

	DX11VertexBuffer::DX11VertexBuffer(uint32_t aCount, uint32_t aStride)
	{
		EPOCH_PROFILE_FUNC();

		myUsage = VertexBufferUsage::Dynamic;
		myCount = aCount;
		myStride = aStride;
		mySize = aCount * aStride;
		myLocalData.Allocate(mySize);

		D3D11_BUFFER_DESC vertexBufferDescription{};
		vertexBufferDescription.ByteWidth = (UINT)mySize;
		vertexBufferDescription.Usage = D3D11_USAGE_DYNAMIC;
		vertexBufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDescription.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;

		const HRESULT result = RHI::GetDevice()->CreateBuffer(&vertexBufferDescription, nullptr, myBuffer.GetAddressOf());
		if (FAILED(result))
		{
			EPOCH_ASSERT(false, "Failed to create dynamic vertex buffer!");
		}
	}

	DX11VertexBuffer::~DX11VertexBuffer()
	{
		//myBuffer->Release();
		myLocalData.Release();
	}

	void DX11VertexBuffer::SetData(void* aBuffer, uint32_t aCount, uint64_t aOffset)
	{
		EPOCH_PROFILE_FUNC();

		uint64_t size = aCount * myStride;
		EPOCH_ASSERT(size <= mySize, "Buffer overflow!");
		EPOCH_ASSERT(myUsage == VertexBufferUsage::Dynamic, "Tried to set the data of a static buffer!");

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

	void DX11VertexBuffer::Resize(uint32_t aCount)
	{
		EPOCH_PROFILE_FUNC();

		EPOCH_ASSERT(myUsage == VertexBufferUsage::Dynamic, "Tried to resize a static buffer!");
		
		myCount = aCount;
		mySize = aCount * myStride;
		myLocalData.Allocate(mySize);
		
		D3D11_BUFFER_DESC vertexBufferDesc{};
		vertexBufferDesc.ByteWidth = (UINT)mySize;
		vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;

		const HRESULT result = RHI::GetDevice()->CreateBuffer(&vertexBufferDesc, nullptr, myBuffer.ReleaseAndGetAddressOf());
		if (FAILED(result))
		{
			EPOCH_ASSERT(false, "Failed to resize vertex buffer!");
		}
	}
}
