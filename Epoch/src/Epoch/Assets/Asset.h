#pragma once
#include "AssetTypes.h"
#include "Epoch/Core/UUID.h"

namespace Epoch
{
	enum class AssetFlag : uint16_t
	{
		None = 0,
		Missing = 1u << 0,	//This is never used
		Invalid = 1u << 1
	};

	using AssetHandle = UUID;

	class Asset
	{
	public:
		virtual ~Asset() = default;

		static AssetType GetStaticType() { return AssetType::None; }
		virtual AssetType GetAssetType() const { return GetStaticType(); }

		AssetHandle GetHandle() const { return myHandle; }
		uint16_t GetFlags() const { return flags; }

		virtual bool operator==(const Asset& aOther) const
		{
			return myHandle == aOther.myHandle;
		}
		
		virtual bool operator!=(const Asset& aOther) const
		{
			return myHandle != aOther.myHandle;
		}

	private:
		bool IsValid() const { return ((flags & (uint16_t)AssetFlag::Missing) | (flags & (uint16_t)AssetFlag::Invalid)) == 0; }

		bool IsFlagSet(AssetFlag aFlag) const { return (uint16_t)aFlag & flags; }
		void SetFlag(AssetFlag aFlag, bool value = true)
		{
			if (value)
			{
				flags |= (uint16_t)aFlag;
			}
			else
			{
				flags &= ~(uint16_t)aFlag;
			}
		}
	private:
		AssetHandle myHandle;
		uint16_t flags = (uint16_t)AssetFlag::None;

		friend class Scene;
		friend class AssetPack;
		friend class AssetManager;
		friend class EditorAssetManager;
		friend class SceneAssetSerializer;
		friend class TextureSerializer;
		friend class FontSerializer;
		friend class EnvironmentSerializer;
		friend class PrefabSerializer;
		friend class MeshSerializer;
		friend class MaterialSerializer;
		friend class PhysicsMaterialSerializer;
		friend class ScriptFileSerializer;
	};
}
