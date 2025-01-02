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
			std::vector<AssetHandle> submeshes;

			std::string name;
			CU::Matrix4x4f localTransform;

			inline bool IsRoot() const { return parent == 0xffffffff; }
		};

	public:
		Model() = default;
		~Model() = default;

		static AssetType GetStaticType() { return AssetType::Model; }
		AssetType GetAssetType() const override { return GetStaticType(); }

	private:
		std::string name;

		std::vector<Node> myNodes;
	};
}
