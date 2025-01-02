#pragma once
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <CommonUtilities/Color.h>
#include <CommonUtilities/Math/Transform.h>
#include "Epoch/Debug/Log.h"
#include "Epoch/Assets/Asset.h"
#include "Epoch/Math/AABB.h"

namespace Epoch
{
	class VertexBuffer;
	class IndexBuffer;
	class Skeleton;

	struct Vertex
	{
		CU::Vector3f position = CU::Vector4f::Zero;
		CU::Vector3f normal = CU::Vector3f::Zero;
		CU::Vector3f tangent = CU::Vector3f::Zero;
		CU::Vector2f uv = CU::Vector2f::Zero;
		CU::Vector3f color = CU::Color::White.GetVector3();

		Vertex() = default;
		Vertex(float x, float y, float z,
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u = 0.0f, float v = 0.0f,
			float r = 1.0f, float g = 1.0f, float b = 1.0f)
		{
			position = { x, y, z };
			normal = { nx, ny, nz };
			tangent = { tx, ty, tz };
			uv = { u, v };
			color = { r, g, b };
		}

		Vertex(CU::Vector3f aPos,
			CU::Vector3f aNormals,
			CU::Vector3f aTangent)
		{
			position = aNormals;
			normal = aNormals;
			tangent = aTangent;
		}
	};

	typedef uint32_t Index;

	//struct Triangle
	//{
	//	Vertex vertex0;
	//	Vertex vertex1;
	//	Vertex vertex2;
	//
	//	Triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2) : vertex0(v0), vertex1(v1), vertex2(v2) {}
	//};

	//struct BoneInfluence
	//{
	//	uint32_t boneIndices[4] = { 0, 0, 0, 0 };
	//	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	//
	//	void AddBoneData(uint32_t aBoneIndex, float aWeight)
	//	{
	//		if (aWeight < 0.0f || aWeight > 1.0f)
	//		{
	//			LOG_WARNING("Vertex bone weight is out of range. We will clamp it to [0, 1] (BoneID: {}, Weight: {})", aBoneIndex, aWeight);
	//			aWeight = std::clamp(aWeight, 0.0f, 1.0f);
	//		}
	//		if (aWeight > 0.0f)
	//		{
	//			for (size_t i = 0; i < 4; i++)
	//			{
	//				if (weights[i] == 0.0f)
	//				{
	//					boneIndices[i] = aBoneIndex;
	//					weights[i] = aWeight;
	//					return;
	//				}
	//			}
	//
	//			LOG_WARNING("Vertex has more than four bones affecting it, extra bone influences will be discarded (BoneID: {}, Weight: {})", aBoneIndex, aWeight);
	//		}
	//	}
	//
	//	void NormalizeWeights()
	//	{
	//		float sumWeights = 0.0f;
	//		for (size_t i = 0; i < 4; i++)
	//		{
	//			sumWeights += weights[i];
	//		}
	//		if (sumWeights > 0.0f)
	//		{
	//			for (size_t i = 0; i < 4; i++)
	//			{
	//				weights[i] /= sumWeights;
	//			}
	//		}
	//	}
	//};

	struct Submesh
	{
		uint32_t materialIndex = 0;

		CU::Matrix4x4f transform; // World transform
		CU::Matrix4x4f localTransform;

		std::string submeshName = "";
	};

	class Mesh : public Asset
	{
	public:
		Mesh() = default;
		Mesh(const std::vector<Vertex>& aVertices, const std::vector<Index>& aIndices);
		~Mesh() = default;

		static AssetType GetStaticType() { return AssetType::Mesh; }
		AssetType GetAssetType() const override { return GetStaticType(); }

		std::shared_ptr<VertexBuffer> GetVertexBuffer() { return myVertexBuffer; }
		std::shared_ptr<IndexBuffer> GetIndexBuffer() { return myIndexBuffer; }
		//std::shared_ptr<VertexBuffer> GetBoneInfluenceBuffer() { return myBoneInfluenceBuffer; }

		//const std::vector<Triangle>& GetTriangleCache(uint32_t index) const { return myTriangleCache.at(index); }
		uint32_t GetTriangleCount() { return (uint32_t)myIndices.size() / 3u; }
		uint32_t GetVertexCount() { return (uint32_t)myVertices.size(); }
		uint32_t GetIndexCount() { return (uint32_t)myIndices.size(); }

		const AABB& GetBoundingBox() const { return myBoundingBox; }

		uint32_t GetMaterialIndex() const { return myMaterialIndex; }
		//uint32_t GetMaterialCount() { return myMaterialCount; }
		//const std::vector<std::string>& GetMaterialNames() { return myMaterialNames; }

		const CU::Matrix4x4f& GetTransform() const { return myLocalTransform; }

	private:
		std::shared_ptr<VertexBuffer> myVertexBuffer;
		std::shared_ptr<IndexBuffer> myIndexBuffer;
		//std::shared_ptr<VertexBuffer> myBoneInfluenceBuffer;

		std::vector<Vertex> myVertices;
		std::vector<Index> myIndices;

		uint32_t myVertexCount;
		uint32_t myIndexCount;

		//std::unordered_map<uint32_t, std::vector<Triangle>> myTriangleCache;

		AABB myBoundingBox;

		uint32_t myMaterialIndex = 0;

		CU::Matrix4x4f myTransform; // World transform
		CU::Matrix4x4f myLocalTransform;

		std::string myName = "";

		//uint32_t myMaterialCount = 0;
		//std::vector<std::string> myMaterialNames;

		friend class AssimpModelImporter;
	};
}
