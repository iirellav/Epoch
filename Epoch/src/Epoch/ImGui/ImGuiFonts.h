#pragma once
#include <imgui/imgui.h>
#include <string>

namespace Epoch::UI
{
	struct FontConfiguration
	{
		std::string fontName;
		std::string_view filePath;
		float size = 18.0f;
		const ImWchar* glyphRanges = nullptr;
		bool mergeWithLast = false;
	};

	class Fonts
	{
	public:
		static void Add(const std::string& aFontName, const std::string_view& aFilePath, bool aIsDefault = false);
		static void Add(const FontConfiguration& aConfig, bool aIsDefault = false);
		static void PushFont(const std::string& aFontName);
		static void PopFont();
		static ImFont* Get(const std::string& aFontName);
	};
}
