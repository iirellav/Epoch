#include "epch.h"
#include "ImGuiFonts.h"

namespace Epoch::UI
{
	static std::unordered_map<std::string, ImFont*> staticFonts;

	void Fonts::Add(const std::string& aFontName, const std::string_view& aFilePath, bool aIsDefault)
	{
		FontConfiguration config;
		config.fontName = aFontName;
		config.filePath = aFilePath;
		Add(config, aIsDefault);
	}

	void Fonts::Add(const FontConfiguration& aConfig, bool aIsDefault)
	{
		if (staticFonts.find(aConfig.fontName) != staticFonts.end())
		{
			LOG_WARNING("Tried to add font with name '{}' but that name is already taken!", aConfig.fontName);
			return;
		}

		ImFontConfig imguiFontConfig;
		imguiFontConfig.MergeMode = aConfig.mergeWithLast;
		auto& io = ImGui::GetIO();
		ImFont* font = io.Fonts->AddFontFromFileTTF(aConfig.filePath.data(), aConfig.size, &imguiFontConfig, aConfig.glyphRanges == nullptr ? io.Fonts->GetGlyphRangesDefault() : aConfig.glyphRanges);
		EPOCH_ASSERT(font, "Failed to load font file!");
		staticFonts[aConfig.fontName] = font;

		if (aIsDefault)
		{
			io.FontDefault = font;
		}
	}

	void Fonts::PushFont(const std::string& aFontName)
	{
		if (staticFonts.find(aFontName) == staticFonts.end())
		{
			const auto& io = ImGui::GetIO();
			ImGui::PushFont(io.FontDefault);
			return;
		}

		ImGui::PushFont(staticFonts.at(aFontName));
	}

	void Fonts::PopFont()
	{
		ImGui::PopFont();
	}

	ImFont* Fonts::Get(const std::string& aFontName)
	{
		EPOCH_ASSERT(staticFonts.find(aFontName) != staticFonts.end(), "Failed to find font with that name!");
		return staticFonts.at(aFontName);
	}
}
