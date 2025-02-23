#pragma once
#include <memory>
#include <filesystem>
#include "Epoch/Rendering/Mesh.h"

namespace Epoch
{
#ifdef _RUNTIME
	class AssimpMeshImporter
	{
	public:
		AssimpMeshImporter(const std::filesystem::path& aFilepath) {}

		std::shared_ptr<Mesh> ImportMesh() { return nullptr; }

	private:
		void TraverseNodes(std::shared_ptr<Mesh> aMesh, void* aAssimpNode, uint32_t aNodeIndex, const CU::Matrix4x4f& parentTransform = CU::Matrix4x4f::Identity, uint32_t aLevel = 0) {}

	private:
		std::filesystem::path myPath;
	};
#else
	class AssimpMeshImporter
	{
	public:
		AssimpMeshImporter(const std::filesystem::path& aFilepath);
		
		std::shared_ptr<Mesh> ImportMesh();
		
	private:
		void TraverseNodes(std::shared_ptr<Mesh> aMesh, void* aAssimpNode, uint32_t aNodeIndex, const CU::Matrix4x4f& parentTransform = CU::Matrix4x4f::Identity, uint32_t aLevel = 0);

	private:
		std::filesystem::path myPath;
	};
#endif
}
