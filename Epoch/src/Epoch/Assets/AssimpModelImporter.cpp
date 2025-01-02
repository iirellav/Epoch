#include "epch.h"
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <CommonUtilities/Math/Transform.h>
#include "AssimpModelImporter.h"
#include "Epoch/Utils/AssimpLogStream.h"
#include "Epoch/Rendering/VertexBuffer.h"
#include "Epoch/Rendering/IndexBuffer.h"
#include "Epoch/Rendering/RendererAPI.h"
#include "Epoch/Rendering/Mesh.h"
#include "Epoch/Assets/AssetManager.h"

namespace Epoch
{
	static const uint32_t staticMeshImportFlags =
		aiProcess_CalcTangentSpace |        // Create binormals/tangents just in case
		aiProcess_Triangulate |             // Make sure we're triangles
		aiProcess_SortByPType |             // Split meshes by primitive type
		aiProcess_GenNormals |              // Make sure we have legit normals
		aiProcess_GenUVCoords |             // Convert UVs if required 
		//aiProcess_OptimizeGraph |
		aiProcess_OptimizeMeshes |          // Batch draws where possible
		//aiProcess_FindInstances |         // Removes duplicate meshes by referencing the same mesh
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

	AssimpModelImporter::AssimpModelImporter(const std::filesystem::path& aFilepath) : myPath(aFilepath)
	{
#ifdef _DEBUG
		//AssimpLogStream::Initialize();
#endif
	}

	std::shared_ptr<Model> AssimpModelImporter::ImportModel(AssetHandle aModelHandle)
	{
		EPOCH_PROFILE_FUNC();

		std::shared_ptr<Model> modelAsset = std::make_shared<Model>();

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
			return modelAsset;
		}
		
		if (scene->HasMeshes())
		{
			myMeshes.reserve(scene->mNumMeshes);
			for (unsigned m = 0; m < scene->mNumMeshes; m++)
			{
				aiMesh* aMesh = scene->mMeshes[m];

				EPOCH_ASSERT(aMesh->HasPositions(), "Meshes require positions.");
				EPOCH_ASSERT(aMesh->HasNormals(), "Meshes require normals.");

				std::shared_ptr<Mesh> meshAsset = std::make_shared<Mesh>();
				meshAsset->myHandle = aModelHandle + AssetHandle(m + 1); //TODO: Maybe the asset handle should be random and saved in the metadata. This would mean that mmodels would need a uniqe metadata structure, and by that point the metadata could be unique for every asset type.
				meshAsset->myMaterialIndex = aMesh->mMaterialIndex;
				meshAsset->myVertexCount = aMesh->mNumVertices;
				meshAsset->myIndexCount = aMesh->mNumFaces * 3;

				auto& aabb = meshAsset->myBoundingBox;
				aabb.min = { FLT_MAX, FLT_MAX, FLT_MAX };
				aabb.max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
				for (uint32_t i = 0; i < aMesh->mNumVertices; i++)
				{
					Vertex vertex;
					vertex.position = { aMesh->mVertices[i].x, aMesh->mVertices[i].y, aMesh->mVertices[i].z };
					vertex.normal = { aMesh->mNormals[i].x, aMesh->mNormals[i].y, aMesh->mNormals[i].z };

					aabb.min.x = CU::Math::Min(vertex.position.x, aabb.min.x);
					aabb.min.y = CU::Math::Min(vertex.position.y, aabb.min.y);
					aabb.min.z = CU::Math::Min(vertex.position.z, aabb.min.z);
					aabb.max.x = CU::Math::Max(vertex.position.x, aabb.max.x);
					aabb.max.y = CU::Math::Max(vertex.position.y, aabb.max.y);
					aabb.max.z = CU::Math::Max(vertex.position.z, aabb.max.z);

					if (aMesh->HasTangentsAndBitangents())
					{
						vertex.tangent = { aMesh->mTangents[i].x, aMesh->mTangents[i].y, aMesh->mTangents[i].z };
					}

					if (aMesh->HasTextureCoords(0))
					{
						vertex.uv = { aMesh->mTextureCoords[0][i].x, aMesh->mTextureCoords[0][i].y };
					}

					if (aMesh->HasVertexColors(0))
					{
						vertex.color = { aMesh->mColors[0][i].r, aMesh->mColors[0][i].g, aMesh->mColors[0][i].b };
					}

					meshAsset->myVertices.push_back(vertex);
				}

				for (uint32_t i = 0; i < aMesh->mNumFaces; i++)
				{
					EPOCH_ASSERT(aMesh->mFaces[i].mNumIndices == 3, "A face must have 3 indices!");

					meshAsset->myIndices.push_back(aMesh->mFaces[i].mIndices[0]);
					meshAsset->myIndices.push_back(aMesh->mFaces[i].mIndices[1]);
					meshAsset->myIndices.push_back(aMesh->mFaces[i].mIndices[2]);

					//meshAsset->myTriangleCache[m].emplace_back
					//(
					//	meshAsset->myVertices[mesh->mFaces[i].mIndices[0]],
					//	meshAsset->myVertices[mesh->mFaces[i].mIndices[1]],
					//	meshAsset->myVertices[mesh->mFaces[i].mIndices[2]]
					//);
				}

				myMeshes.push_back(meshAsset);
			}

			Model::Node& rootNode = modelAsset->myNodes.emplace_back();
			TraverseNodes(modelAsset, scene->mRootNode, 0);

			for (const auto& mesh : myMeshes)
			{
				AABB transformedMeshAABB = mesh->myBoundingBox;
				CU::Vector3f min = CU::Vector3f(mesh->myTransform * CU::Vector4f(transformedMeshAABB.min, 1.0f));
				CU::Vector3f max = CU::Vector3f(mesh->myTransform * CU::Vector4f(transformedMeshAABB.max, 1.0f));

				mesh->myBoundingBox.min.x = CU::Math::Min(mesh->myBoundingBox.min.x, min.x);
				mesh->myBoundingBox.min.y = CU::Math::Min(mesh->myBoundingBox.min.y, min.y);
				mesh->myBoundingBox.min.z = CU::Math::Min(mesh->myBoundingBox.min.z, min.z);
				mesh->myBoundingBox.max.x = CU::Math::Max(mesh->myBoundingBox.max.x, max.x);
				mesh->myBoundingBox.max.y = CU::Math::Max(mesh->myBoundingBox.max.y, max.y);
				mesh->myBoundingBox.max.z = CU::Math::Max(mesh->myBoundingBox.max.z, max.z);
			}
		}

		//meshAsset->myMaterialCount = scene->mNumMaterials;
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

		for (const auto& mesh : myMeshes)
		{
			if (mesh->myVertices.size())
			{
				mesh->myVertexBuffer = VertexBuffer::Create(mesh->myVertices.data(), (uint32_t)mesh->myVertices.size(), sizeof(Vertex));
			}

			if (mesh->myIndices.size())
			{
				mesh->myIndexBuffer = IndexBuffer::Create(mesh->myIndices.data(), (uint32_t)mesh->myIndices.size());
			}

			AssetManager::AddMemoryOnlyAssetWithHandle<Mesh>(mesh->GetHandle(), mesh, mesh->myName);
		}

		return modelAsset;
	}

	void AssimpModelImporter::TraverseNodes(std::shared_ptr<Model> aModel, void* aAssimpNode, uint32_t aNodeIndex, const CU::Matrix4x4f& parentTransform, uint32_t aLevel)
	{
		aiNode* aNode = (aiNode*)aAssimpNode;

		Model::Node& node = aModel->myNodes[aNodeIndex];
		node.name = aNode->mName.C_Str();
		node.localTransform = Mat4FromAIMat4(aNode->mTransformation);

		const CU::Matrix4x4f transform = parentTransform * node.localTransform;
		//const CU::Matrix4x4f transform = node.localTransform * parentTransform;
		for (uint32_t i = 0; i < aNode->mNumMeshes; i++)
		{
			uint32_t meshIndex = aNode->mMeshes[i];
			auto& mesh = myMeshes[meshIndex];
			mesh->myName = aNode->mName.C_Str();
			mesh->myTransform = transform;
			mesh->myLocalTransform = node.localTransform;

			node.meshes.push_back(mesh->GetHandle());
		}

		uint32_t parentNodeIndex = (uint32_t)aModel->myNodes.size() - 1;
		node.children.resize(aNode->mNumChildren);
		for (uint32_t i = 0; i < aNode->mNumChildren; i++)
		{
			Model::Node& child = aModel->myNodes.emplace_back();
			size_t childIndex = aModel->myNodes.size() - 1;
			child.parent = parentNodeIndex;
			aModel->myNodes[aNodeIndex].children[i] = (uint32_t)childIndex;
			TraverseNodes(aModel, aNode->mChildren[i], uint32_t(childIndex), transform, aLevel + 1);
		}
	}
}
