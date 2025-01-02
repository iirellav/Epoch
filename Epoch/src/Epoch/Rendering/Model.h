#pragma once
#include <string>
#include <vector>
#include <CommonUtilities/Math/Matrix/Matrix4x4.hpp>
#include "Epoch/Assets/Asset.h"

namespace Epoch
{
	class Model : public Asset
	{
	public:
		struct Node
		{
			uint32_t parent = 0xffffffff;
			std::vector<uint32_t> children;
			std::vector<AssetHandle> meshes;

			std::string name;
			CU::Matrix4x4f localTransform;

			inline bool IsRoot() const { return parent == 0xffffffff; }
		};

	public:
		Model() = default;
		~Model() = default;

		static AssetType GetStaticType() { return AssetType::Model; }
		AssetType GetAssetType() const override { return GetStaticType(); }

		const Node& GetRootNode() const { return myNodes[0]; }
		const std::vector<Node>& GetNodes() const { return myNodes; }

	private:
		std::string myName;

		std::vector<Node> myNodes;

		friend class AssimpModelImporter;
	};
}
