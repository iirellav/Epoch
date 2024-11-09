#pragma once
#include "Epoch/Assets/Asset.h"

namespace Epoch
{
	class ScriptAsset : public Asset
	{
	public:
		ScriptAsset() = default;
		ScriptAsset(uint32_t aClassID) : myClassID(aClassID) {}
		~ScriptAsset() override = default;
		
		uint32_t GetClassID() const { return myClassID; }

		static AssetType GetStaticType() { return AssetType::Script; }
		virtual AssetType GetAssetType() const { return GetStaticType(); }

	private:
		uint32_t myClassID = 0;
	};

	class ScriptFileAsset : public Asset
	{
	public:
		ScriptFileAsset() = default;
		ScriptFileAsset(const char* aClassNamespace, const char* aClassName) : myClassNamespace(aClassNamespace), myClassName(aClassName) {}

		const std::string& GetScriptNamespace() const { return myClassNamespace; }
		const std::string& GetScriptName() const { return myClassName; }

		static AssetType GetStaticType() { return AssetType::ScriptFile; }
		virtual AssetType GetAssetType() const override { return GetStaticType(); }

	private:
		std::string myClassNamespace = "";
		std::string myClassName = "";
	};
}
