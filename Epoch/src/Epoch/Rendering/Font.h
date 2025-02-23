#pragma once
#include <memory>
#include <filesystem>
#include "Epoch/Assets/Asset.h"
#include "Epoch/Rendering/Texture.h"

namespace Epoch
{
	struct MSDFData;

	class Font : public Asset
	{
	public:
		Font(const std::filesystem::path& aFilepath);
		Font(const std::string& aName, Buffer aBuffer);
		~Font() override = default;

		std::shared_ptr<Texture2D> GetFontAtlas() const { return myTextureAtlas; }
		const MSDFData* GetMSDFData() const { return myMSDFData; }

		static void Init();
		static void Shutdown();
		static std::shared_ptr<Font> GetDefaultFont() { return staticDefaultFont; }

		static AssetType GetStaticType() { return AssetType::Font; }
		AssetType GetAssetType() const override { return GetStaticType(); }

		const std::string& GetName() const { return myName; }

	private:
		void CreateAtlas(Buffer aBuffer);

	private:
		std::string myName;
		std::shared_ptr<Texture2D> myTextureAtlas;
		MSDFData* myMSDFData = nullptr;

		inline static std::shared_ptr<Font> staticDefaultFont;
	};
}
