#include "epch.h"
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include "AssimpMeshImporter.h"
#include <CommonUtilities/Math/Transform.h>
#include "Epoch/Utils/AssimpLogStream.h"
#include "Epoch/Rendering/VertexBuffer.h"
#include "Epoch/Rendering/IndexBuffer.h"
#include "Epoch/Rendering/RendererAPI.h"
#include "Epoch/Animation/Skeleton.h"

namespace Epoch
{
	static const uint32_t staticMeshImportFlags =
		aiProcess_CalcTangentSpace |        // Create binormals/tangents just in case
		aiProcess_Triangulate |             // Make sure we're triangles
		aiProcess_SortByPType |             // Split meshes by primitive type
		aiProcess_GenNormals |              // Make sure we have legit normals
		aiProcess_GenUVCoords |             // Convert UVs if required 
		aiProcess_GenBoundingBoxes |
		//aiProcess_OptimizeGraph |
		aiProcess_OptimizeMeshes |          // Batch draws where possible
		aiProcess_JoinIdenticalVertices |
		aiProcess_LimitBoneWeights |        // If more than N (=4) bone weights, discard least influencing bones and renormalise sum to 1
		aiProcess_ValidateDataStructure     // Validation
		//| aiProcess_GlobalScale           // e.g. convert cm to m for fbx import (and other formats where cm is native)
		;

	CU::Matrix4x4f Mat4FromAIMat4(const aiMatrix4x4& matrix)
	{
		CU::Matrix4x4f result;
		result[0] = matrix.a1; result[4] = matrix.a2; result[8] = matrix.a3; result[12] = matrix.a4;
		result[1] = matrix.b1; result[5] = matrix.b2; result[9] = matrix.b3; result[13] = matrix.b4;
		result[2] = matrix.c1; result[6] = matrix.c2; result[10] = matrix.c3; result[14] = matrix.c4;
		result[3] = matrix.d1; result[7] = matrix.d2; result[11] = matrix.d3; result[15] = matrix.d4;
		return result;
	}

	class BoneHierarchy
	{
	public:
		BoneHierarchy(const aiScene* aScene);

		void ExtractBones();
		void TraverseNode(aiNode* aNode, Skeleton* aSkeleton);
		uint32_t TraverseBone(aiNode* aNode, Skeleton* aSkeleton, uint32_t aParentIndex);
		std::shared_ptr<Skeleton> CreateSkeleton();

	private:
		std::set<std::string_view> myBones;
		const aiScene* myScene;
	};

	BoneHierarchy::BoneHierarchy(const aiScene* aScene) : myScene(aScene) {}

	void BoneHierarchy::ExtractBones()
	{
		// Note: ASSIMP does not appear to support import of digital content files that contain _only_ an armature/skeleton and no mesh.
		for (uint32_t meshIndex = 0; meshIndex < myScene->mNumMeshes; ++meshIndex)
		{
			const aiMesh* mesh = myScene->mMeshes[meshIndex];
			for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
			{
				myBones.emplace(mesh->mBones[boneIndex]->mName.C_Str());
			}
		}
	}

	void BoneHierarchy::TraverseNode(aiNode* aNode, Skeleton* aSkeleton)
	{
		if (myBones.find(aNode->mName.C_Str()) != myBones.end())
		{
			TraverseBone(aNode, aSkeleton, Skeleton::NullIndex);
		}
		else
		{
			for (uint32_t nodeIndex = 0; nodeIndex < aNode->mNumChildren; ++nodeIndex)
			{
				TraverseNode(aNode->mChildren[nodeIndex], aSkeleton);
			}
		}
	}

	uint32_t BoneHierarchy::TraverseBone(aiNode* aNode, Skeleton* aSkeleton, uint32_t aParentIndex)
	{
		uint32_t boneIndex = aSkeleton->AddBone(aNode->mName.C_Str(), aParentIndex, Mat4FromAIMat4(aNode->mTransformation));
		for (uint32_t nodeIndex = 0; nodeIndex < aNode->mNumChildren; ++nodeIndex)
		{
			uint32_t childBoneIndex = TraverseBone(aNode->mChildren[nodeIndex], aSkeleton, boneIndex);
			aSkeleton->AddBoneChild(boneIndex, childBoneIndex);
		}

		return boneIndex;
	}

	std::shared_ptr<Skeleton> BoneHierarchy::CreateSkeleton()
	{
		if (!myScene)
		{
			return nullptr;
		}

		ExtractBones();
		if (myBones.empty())
		{
			return nullptr;
		}

		auto skeleton = std::make_shared<Skeleton>(static_cast<uint32_t>(myBones.size()));
		TraverseNode(myScene->mRootNode, skeleton.get());

		return skeleton;
	}

	AssimpMeshImporter::AssimpMeshImporter(const std::filesystem::path& aFilepath) : myPath(aFilepath)
	{
#ifdef _DEBUG
		//AssimpLogStream::Initialize();
#endif
	}

	std::shared_ptr<Mesh> AssimpMeshImporter::ImportMesh()
	{
		EPOCH_PROFILE_FUNC();

		std::shared_ptr<Mesh> meshAsset = std::make_shared<Mesh>();

		Assimp::Importer importer;
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

		uint32_t importFlags = 0;;
		if (RendererAPI::Current() == RendererAPIType::DirectX11 || RendererAPI::Current() == RendererAPIType::DirectX12)
		{
			importFlags |= aiProcess_ConvertToLeftHanded;
		}
		importFlags |= staticMeshImportFlags;

		const aiScene* scene = importer.ReadFile(myPath.string(), importFlags);
		if (!scene)
		{
			LOG_ERROR("Failed to load mesh file: {}", myPath.string());
			return meshAsset;
		}

		BoneHierarchy boneHierarchy(scene);
		meshAsset->mySkeleton = boneHierarchy.CreateSkeleton();
		meshAsset->myAnimationCount = scene->mNumAnimations;
		
		if (scene->HasMeshes())
		{
			uint32_t vertexCount = 0;
			uint32_t indexCount = 0;
			
			meshAsset->mySubmeshes.reserve(scene->mNumMeshes);
			for (unsigned m = 0; m < scene->mNumMeshes; m++)
			{
				aiMesh* mesh = scene->mMeshes[m];

				EPOCH_ASSERT(mesh->HasPositions(), "Meshes require positions.");
				EPOCH_ASSERT(mesh->HasNormals(), "Meshes require normals.");

				Submesh& submesh = meshAsset->mySubmeshes.emplace_back();
				submesh.baseVertex = vertexCount;
				submesh.baseIndex = indexCount;
				submesh.materialIndex = mesh->mMaterialIndex;
				submesh.vertexCount = mesh->mNumVertices;
				submesh.indexCount = mesh->mNumFaces * 3;
				submesh.meshName = mesh->mName.C_Str();

				vertexCount += submesh.vertexCount;
				indexCount += submesh.indexCount;

				for (uint32_t i = 0; i < mesh->mNumVertices; i++)
				{
					Vertex vertex;
					vertex.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
					vertex.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

					if (mesh->HasTangentsAndBitangents())
					{
						vertex.tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
					}

					if (mesh->HasTextureCoords(0))
					{
						vertex.uv = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
					}

					if (mesh->HasVertexColors(0))
					{
						vertex.color = { mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b };
					}

					meshAsset->myVertices.push_back(vertex);
				}

				for (uint32_t i = 0; i < mesh->mNumFaces; i++)
				{
					EPOCH_ASSERT(mesh->mFaces[i].mNumIndices == 3, "A face must have 3 indices!");

					meshAsset->myIndices.push_back(mesh->mFaces[i].mIndices[0]);
					meshAsset->myIndices.push_back(mesh->mFaces[i].mIndices[1]);
					meshAsset->myIndices.push_back(mesh->mFaces[i].mIndices[2]);

					meshAsset->myTriangleCache[m].emplace_back
					(
						meshAsset->myVertices[mesh->mFaces[i].mIndices[0]],
						meshAsset->myVertices[mesh->mFaces[i].mIndices[1]],
						meshAsset->myVertices[mesh->mFaces[i].mIndices[2]]
					);
				}
			}

			MeshNode& rootNode = meshAsset->myNodes.emplace_back();
			TraverseNodes(meshAsset, scene->mRootNode, 0);
		}

		if (meshAsset->HasSkeleton())
		{
			meshAsset->myBoneInfluences.resize(meshAsset->myVertices.size());
			for (uint32_t m = 0; m < scene->mNumMeshes; m++)
			{
				aiMesh* mesh = scene->mMeshes[m];
				Submesh& submesh = meshAsset->mySubmeshes[m];

				if (mesh->mNumBones > 0)
				{
					for (uint32_t i = 0; i < mesh->mNumBones; i++)
					{
						aiBone* bone = mesh->mBones[i];

						bool hasNonZeroWeight = false;
						for (size_t j = 0; j < bone->mNumWeights; j++)
						{
							if (bone->mWeights[j].mWeight > 0.000001f)
							{
								hasNonZeroWeight = true;
							}
						}
						if (!hasNonZeroWeight)
						{
							continue;
						}

						// Find bone in skeleton
						uint32_t boneIndex = meshAsset->mySkeleton->GetBoneIndex(bone->mName.C_Str());
						if (boneIndex == Skeleton::NullIndex)
						{
							EPOCH_ASSERT("Could not find mesh bone '{}' in skeleton!", bone->mName.C_Str());
						}

						auto& b = meshAsset->GetSkeleton()->GetBone(boneIndex);
						b.invBindPose = Mat4FromAIMat4(bone->mOffsetMatrix);

						for (size_t j = 0; j < bone->mNumWeights; j++)
						{
							int VertexID = bone->mWeights[j].mVertexId;
							float Weight = bone->mWeights[j].mWeight;
							meshAsset->myBoneInfluences[VertexID].AddBoneData(boneIndex, Weight);
						}
					}
				}
			}

			for (auto& boneInfluence : meshAsset->myBoneInfluences)
			{
				boneInfluence.NormalizeWeights();
			}
		}
		
		meshAsset->myMaterialCount = scene->mNumMaterials;
		//if (scene->HasMaterials())
		//{
		//	meshAsset->myMaterialCount = scene->mNumMaterials;
		//
		//	for (uint32_t i = 0; i < scene->mNumMaterials; i++)
		//	{
		//		auto aiMaterial = scene->mMaterials[i];
		//		auto aiMaterialName = aiMaterial->GetName();
		//		meshAsset->myMaterialNames.push_back(aiMaterialName.data);
		//	}
		//}

		if (meshAsset->myVertices.size())
		{
			meshAsset->myVertexBuffer = VertexBuffer::Create(meshAsset->myVertices.data(), (uint32_t)meshAsset->myVertices.size(), sizeof(Vertex));
		}

		if (meshAsset->myBoneInfluences.size())
		{
			meshAsset->myBoneInfluenceBuffer = VertexBuffer::Create(meshAsset->myBoneInfluences.data(), (uint32_t)meshAsset->myBoneInfluences.size(), sizeof(BoneInfluence));
		}

		if (meshAsset->myIndices.size())
		{
			meshAsset->myIndexBuffer = IndexBuffer::Create(meshAsset->myIndices.data(), (uint32_t)meshAsset->myIndices.size());
		}

		return meshAsset;
	}

	void AssimpMeshImporter::TraverseNodes(std::shared_ptr<Mesh> aMesh, void* aAssimpNode, uint32_t aNodeIndex, const CU::Matrix4x4f& parentTransform, uint32_t aLevel)
	{
		aiNode* aNode = (aiNode*)aAssimpNode;

		MeshNode& node = aMesh->myNodes[aNodeIndex];
		node.name = aNode->mName.C_Str();
		node.localTransform = Mat4FromAIMat4(aNode->mTransformation);

		const CU::Matrix4x4f transform = parentTransform * node.localTransform;
		//const CU::Matrix4x4f transform = node.localTransform * parentTransform;
		for (uint32_t i = 0; i < aNode->mNumMeshes; i++)
		{
			uint32_t submeshIndex = aNode->mMeshes[i];
			auto& submesh = aMesh->mySubmeshes[submeshIndex];
			submesh.nodeName = aNode->mName.C_Str();
			submesh.transform = transform;
			submesh.localTransform = node.localTransform;

			node.submeshes.push_back(submeshIndex);
		}

		uint32_t parentNodeIndex = (uint32_t)aMesh->myNodes.size() - 1;
		node.children.resize(aNode->mNumChildren);
		for (uint32_t i = 0; i < aNode->mNumChildren; i++)
		{
			MeshNode& child = aMesh->myNodes.emplace_back();
			size_t childIndex = aMesh->myNodes.size() - 1;
			child.parent = parentNodeIndex;
			aMesh->myNodes[aNodeIndex].children[i] = (uint32_t)childIndex;
			TraverseNodes(aMesh, aNode->mChildren[i], uint32_t(childIndex), transform, aLevel + 1);
		}
	}
}
