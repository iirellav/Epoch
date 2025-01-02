#pragma once
#include <memory>
#include <filesystem>
#include "Epoch/Rendering/Model.h"

namespace Epoch
{
	class Mesh;

	class AssimpModelImporter
	{
	public:
		AssimpModelImporter(const std::filesystem::path& aFilepath);
		
		std::shared_ptr<Model> ImportModel(AssetHandle aModelHandle);
		
	private:
		void TraverseNodes(std::shared_ptr<Model> aModel, void* aAssimpNode, uint32_t aNodeIndex, const CU::Matrix4x4f& parentTransform = CU::Matrix4x4f::Identity, uint32_t aLevel = 0);

	private:
		std::filesystem::path myPath;
		std::vector<std::shared_ptr<Mesh>> myMeshes;
	};
}
