#pragma once
#include <Epoch/Assets/Asset.h>

namespace Epoch
{
	class SelectionStack
	{
	public:
		void CopyFrom(const SelectionStack& aOther)
		{
			mySelections.assign(aOther.begin(), aOther.end());
		}

		void CopyFrom(const std::vector<AssetHandle>& aOther)
		{
			mySelections.assign(aOther.begin(), aOther.end());
		}

		void Select(AssetHandle aHandle)
		{
			if (IsSelected(aHandle))
			{
				return;
			}

			mySelections.push_back(aHandle);
		}

		void Deselect(AssetHandle aHandle)
		{
			if (!IsSelected(aHandle))
			{
				return;
			}

			for (auto it = mySelections.begin(); it != mySelections.end(); it++)
			{
				if (aHandle == *it)
				{
					mySelections.erase(it);
					break;
				}
			}
		}

		bool IsSelected(AssetHandle aHandle) const
		{
			for (const auto& selectedHandle : mySelections)
			{
				if (selectedHandle == aHandle)
				{
					return true;
				}
			}

			return false;
		}

		void Clear()
		{
			mySelections.clear();
		}

		size_t SelectionCount() const { return mySelections.size(); }
		const AssetHandle* SelectionData() const { return mySelections.data(); }

		AssetHandle operator[](size_t aIndex) const
		{
			EPOCH_ASSERT(aIndex >= 0 && aIndex < mySelections.size());
			return mySelections[aIndex];
		}

		std::vector<AssetHandle>::iterator begin() { return mySelections.begin(); }
		std::vector<AssetHandle>::const_iterator begin() const { return mySelections.begin(); }
		std::vector<AssetHandle>::iterator end() { return mySelections.end(); }
		std::vector<AssetHandle>::const_iterator end() const { return mySelections.end(); }

	private:
		std::vector<AssetHandle> mySelections;
	};
}
