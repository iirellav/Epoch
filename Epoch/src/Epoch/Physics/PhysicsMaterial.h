#pragma once
#include "Epoch/Assets/Asset.h"

namespace Epoch
{
	class PhysicsMaterial : public Asset
	{
	public:
		PhysicsMaterial() = default;
		~PhysicsMaterial() override = default;
		
		float StaticFriction() const { return myStaticFriction; }
		void StaticFriction(float aValue) { myStaticFriction = aValue; }
		
		float DynamicFriction() const { return myDynamicFriction; }
		void DynamicFriction(float aValue) { myDynamicFriction = aValue; }
		
		float Restitution() const { return myRestitution; }
		void Restitution(float aValue) { myRestitution = aValue; }

		static AssetType GetStaticType() { return AssetType::PhysicsMaterial; }
		AssetType GetAssetType() const override { return GetStaticType(); }

	private:
		float myStaticFriction = 0.8f;
		float myDynamicFriction = 0.7f;
		float myRestitution = 0.1f;
	};
}
