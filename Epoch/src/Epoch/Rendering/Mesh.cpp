#include "epch.h"
#include "Mesh.h"
#include "Epoch/Rendering/VertexBuffer.h"
#include "Epoch/Rendering/IndexBuffer.h"

namespace Epoch
{
	Mesh::Mesh(const std::vector<Vertex>& aVertices, const std::vector<Index>& aIndices)
	{
		myVertices = aVertices;
		myVertexBuffer = VertexBuffer::Create(myVertices.data(), (uint32_t)myVertices.size(), sizeof(Vertex));

		myIndices = aIndices;
		myIndexBuffer = IndexBuffer::Create(myIndices.data(), (uint32_t)myIndices.size());

		auto& submesh = mySubmeshes.emplace_back();
		submesh.vertexCount = (uint32_t)myVertices.size();
		submesh.indexCount = (uint32_t)myIndices.size();

		for (size_t i = 0; i < myIndices.size(); i += 3)
		{
			myTriangleCache[0].emplace_back
			(
				myVertices[myIndices[i + 0]],
				myVertices[myIndices[i + 1]],
				myVertices[myIndices[i + 2]]
			);
		}

		myBoundingBox.min = { FLT_MAX, FLT_MAX, FLT_MAX };
		myBoundingBox.max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

		for (const Vertex& vertex : myVertices)
		{
			myBoundingBox.min.x = CU::Math::Min(vertex.position.x, myBoundingBox.min.x);
			myBoundingBox.min.y = CU::Math::Min(vertex.position.y, myBoundingBox.min.y);
			myBoundingBox.min.z = CU::Math::Min(vertex.position.z, myBoundingBox.min.z);
			myBoundingBox.max.x = CU::Math::Max(vertex.position.x, myBoundingBox.max.x);
			myBoundingBox.max.y = CU::Math::Max(vertex.position.y, myBoundingBox.max.y);
			myBoundingBox.max.z = CU::Math::Max(vertex.position.z, myBoundingBox.max.z);
		}
	}
}
