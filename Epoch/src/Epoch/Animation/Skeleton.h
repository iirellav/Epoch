#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <CommonUtilities/Math/Quaternion.hpp>
#include <CommonUtilities/Math/Vector/Vector3.hpp>

namespace Epoch
{
	class Skeleton
	{
	public:
		static const uint32_t NullIndex = ~0;
		
		struct Bone
		{
			std::string name;
			
			CU::Vector3f position;
			CU::Quatf orientation;
			CU::Vector3f scale;

			CU::Matrix4x4f invBindPose;

			uint32_t parentIndex;
			std::vector<uint32_t> childrenIndices;
		};

	public:
		Skeleton() = default;
		Skeleton(uint32_t aCount);
		~Skeleton() = default;

		uint32_t AddBone(const std::string& aName, uint32_t aParentIndex, const CU::Matrix4x4f& aTransform);
		void AddBoneChild(uint32_t aIndex, uint32_t& aChild);
		Bone& GetBone(uint32_t aIndex) { return myBones[aIndex]; }
		const Bone& GetBone(uint32_t aIndex) const { return myBones[aIndex]; }
		uint32_t GetBoneIndex(const std::string& aName) const
		{
			if (myNameToIndexMap.find(aName) != myNameToIndexMap.end())
			{
				return myNameToIndexMap.at(aName);
			}

			return NullIndex;
		}
		const std::vector<Bone>& GetBones() const { return myBones; }
		uint32_t GetNumBones() const { return static_cast<uint32_t>(myBones.size()); }

	private:
		std::vector<Bone> myBones;
		std::unordered_map<std::string, uint32_t> myNameToIndexMap;
	};
}
