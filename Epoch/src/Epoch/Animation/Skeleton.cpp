#include "epch.h"
#include "Skeleton.h"

namespace Epoch
{
	Skeleton::Skeleton(uint32_t aCount)
	{
		myBones.reserve(aCount);
		myNameToIndexMap.reserve(aCount);
	}

	uint32_t Skeleton::AddBone(const std::string& aName, uint32_t aParentIndex, const CU::Matrix4x4f& aTransform)
	{
		uint32_t index = static_cast<uint32_t>(myBones.size());
		Bone& bone = myBones.emplace_back();

		bone.name = aName;
		bone.parentIndex = aParentIndex;

		CU::Vector3f rotation;
		aTransform.Decompose(bone.position, rotation, bone.scale);
		bone.orientation = CU::Quatf(rotation);

		EPOCH_ASSERT(myNameToIndexMap.find(aName) == myNameToIndexMap.end(), "Same name on multiple bones!");
		myNameToIndexMap[aName] = index;

		return index;
	}

	void Skeleton::AddBoneChild(uint32_t aIndex, uint32_t& aChild)
	{
		myBones[aIndex].childrenIndices.push_back(aChild);
	}
}
