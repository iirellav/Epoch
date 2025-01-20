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
#include "Epoch/Serialization/StreamReader.h"
#include "Epoch/Serialization/StreamWriter.h"

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
			CU::Vector3f aNormals, //flickvän angry
			CU::Vector3f aTangent)
		{
			position = aNormals;
			normal = aNormals;
			tangent = aTangent;
		}

		static void Serialize(StreamWriter* aStream, const Vertex& aInstance)
		{
			aStream->WriteRaw(aInstance.position);
			aStream->WriteRaw(aInstance.normal);
			aStream->WriteRaw(aInstance.tangent);
			aStream->WriteRaw(aInstance.uv);
			aStream->WriteRaw(aInstance.color);
		}

		static void Deserialize(StreamReader* aStream, Vertex& aInstance)
		{
			aStream->ReadRaw(aInstance.position);
			aStream->ReadRaw(aInstance.normal);
			aStream->ReadRaw(aInstance.tangent);
			aStream->ReadRaw(aInstance.uv);
			aStream->ReadRaw(aInstance.color);
		}
	};

	typedef uint32_t Index;

	struct Triangle
	{
		Vertex vertex0;
		Vertex vertex1;
		Vertex vertex2;

		Triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2) : vertex0(v0), vertex1(v1), vertex2(v2) {}
	};

	struct BoneInfluence
	{
		uint32_t boneIndices[4] = { 0, 0, 0, 0 };
		float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		void AddBoneData(uint32_t aBoneIndex, float aWeight)
		{
			if (aWeight < 0.0f || aWeight > 1.0f)
			{
				LOG_WARNING("Vertex bone weight is out of range. We will clamp it to [0, 1] (BoneID: {}, Weight: {})", aBoneIndex, aWeight);
				aWeight = std::clamp(aWeight, 0.0f, 1.0f);
			}
			if (aWeight > 0.0f)
			{
				for (size_t i = 0; i < 4; i++)
				{
					if (weights[i] == 0.0f)
					{
						boneIndices[i] = aBoneIndex;
						weights[i] = aWeight;
						return;
					}
				}

				LOG_WARNING("Vertex has more than four bones affecting it, extra bone influences will be discarded (BoneID: {}, Weight: {})", aBoneIndex, aWeight);
			}
		}

		void NormalizeWeights()
		{
			float sumWeights = 0.0f;
			for (size_t i = 0; i < 4; i++)
			{
				sumWeights += weights[i];
			}
			if (sumWeights > 0.0f)
			{
				for (size_t i = 0; i < 4; i++)
				{
					weights[i] /= sumWeights;
				}
			}
		}
	};

	struct Submesh
	{
		uint32_t baseVertex = 0;
		uint32_t baseIndex = 0;
		uint32_t materialIndex = 0;
		uint32_t indexCount = 0;
		uint32_t vertexCount = 0;

		CU::Matrix4x4f transform; // World transform
		CU::Matrix4x4f localTransform;
		AABB boundingBox;

		std::string nodeName = "";
		std::string meshName = "";

		static void Serialize(StreamWriter* aStream, const Submesh& aInstance)
		{
			aStream->WriteRaw(aInstance.baseVertex);
			aStream->WriteRaw(aInstance.baseIndex);
			aStream->WriteRaw(aInstance.materialIndex);
			aStream->WriteRaw(aInstance.indexCount);
			aStream->WriteRaw(aInstance.vertexCount);
			aStream->WriteRaw(aInstance.transform);
			aStream->WriteRaw(aInstance.localTransform);
			aStream->WriteRaw(aInstance.boundingBox);
			aStream->WriteString(aInstance.nodeName);
			aStream->WriteString(aInstance.meshName);
		}

		static void Deserialize(StreamReader* aStream, Submesh& aInstance)
		{
			aStream->ReadRaw(aInstance.baseVertex);
			aStream->ReadRaw(aInstance.baseIndex);
			aStream->ReadRaw(aInstance.materialIndex);
			aStream->ReadRaw(aInstance.indexCount);
			aStream->ReadRaw(aInstance.vertexCount);
			aStream->ReadRaw(aInstance.transform);
			aStream->ReadRaw(aInstance.localTransform);
			aStream->ReadRaw(aInstance.boundingBox);
			aStream->ReadString(aInstance.nodeName);
			aStream->ReadString(aInstance.meshName);
		}
	};

	struct MeshNode
	{
		uint32_t parent = 0xffffffff;
		std::vector<uint32_t> children;
		std::vector<uint32_t> submeshes;

		std::string name;
		CU::Matrix4x4f localTransform;

		inline bool IsRoot() const { return parent == 0xffffffff; }

		static void Serialize(StreamWriter* aStream, const MeshNode& aInstance)
		{
			aStream->WriteRaw(aInstance.parent);
			aStream->WriteArray(aInstance.children);
			aStream->WriteArray(aInstance.submeshes);
			aStream->WriteString(aInstance.name);
			aStream->WriteRaw(aInstance.localTransform);
		}

		static void Deserialize(StreamReader* aStream, MeshNode& aInstance)
		{
			aStream->ReadRaw(aInstance.parent);
			aStream->ReadArray(aInstance.children);
			aStream->ReadArray(aInstance.submeshes);
			aStream->ReadString(aInstance.name);
			aStream->ReadRaw(aInstance.localTransform);
		}
	};

	class Mesh : public Asset
	{
	public:
		Mesh() = default;
		Mesh(const std::vector<Vertex>& aVertices, const std::vector<Index>& aIndices);
		~Mesh() = default;

		static AssetType GetStaticType() { return AssetType::Mesh; }
		AssetType GetAssetType() const override { return GetStaticType(); }

		std::vector<Submesh>& GetSubmeshes() { return mySubmeshes; }
		const std::vector<Submesh>& GetSubmeshes() const { return mySubmeshes; }

		std::shared_ptr<VertexBuffer> GetVertexBuffer() { return myVertexBuffer; }
		std::shared_ptr<VertexBuffer> GetBoneInfluenceBuffer() { return myBoneInfluenceBuffer; }
		std::shared_ptr<IndexBuffer> GetIndexBuffer() { return myIndexBuffer; }

		const std::vector<Triangle>& GetTriangleCache(uint32_t index) const { return myTriangleCache.at(index); }
		uint32_t GetTriangleCount() { return (uint32_t)myIndices.size() / 3u; }
		uint32_t GetVertexCount() { return (uint32_t)myVertices.size(); }
		uint32_t GetIndexCount() { return (uint32_t)myIndices.size(); }

		const AABB& GetBoundingBox() const { return myBoundingBox; }

		bool HasSkeleton() const { return (bool)mySkeleton; }
		std::shared_ptr<Skeleton> GetSkeleton() const { return mySkeleton; }

		uint32_t GetAnimationCount() { return myAnimationCount; }

		//uint32_t GetMaterialCount() { return myMaterialCount; }
		//const std::vector<std::string>& GetMaterialNames() { return myMaterialNames; }

		const MeshNode& GetRootNode() const { return myNodes[0]; }
		const std::vector<MeshNode>& GetNodes() const { return myNodes; }

	private:
		std::vector<Submesh> mySubmeshes;

		std::shared_ptr<VertexBuffer> myVertexBuffer;
		std::shared_ptr<VertexBuffer> myBoneInfluenceBuffer;
		std::shared_ptr<IndexBuffer> myIndexBuffer;

		std::vector<Vertex> myVertices;
		std::vector<Index> myIndices;

		std::unordered_map<uint32_t, std::vector<Triangle>> myTriangleCache;

		AABB myBoundingBox;

		std::shared_ptr<Skeleton> mySkeleton;
		std::vector<BoneInfluence> myBoneInfluences;

		uint32_t myAnimationCount = 0;

		uint32_t myMaterialCount = 0;
		//std::vector<std::string> myMaterialNames;

		std::vector<MeshNode> myNodes;

		friend class AssimpMeshImporter;
		friend class SceneRenderer;// TEMP
		friend class MeshRuntimeSerializer;
	};
}
