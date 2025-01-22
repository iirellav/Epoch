#pragma once
#include <imgui/imgui.h>
#include <string>
#include "Epoch/Core/Buffer.h"

namespace Epoch::UI
{
	struct FontConfiguration
	{
		std::string fontName;

		std::string_view filePath;

		//Not owned, not deleted after used
		Buffer data;

		float size = 18.0f;
		const ImWchar* glyphRanges = nullptr;
		bool mergeWithLast = false;
	};

	class Fonts
	{
	public:
		static void Add(const std::string& aFontName, Buffer aBuffer, bool aIsDefault = false);
		static void Add(const std::string& aFontName, const std::string_view& aFilePath, bool aIsDefault = false);
		static void Add(const FontConfiguration& aConfig, bool aIsDefault = false);
		static void PushFont(const std::string& aFontName);
		static void PopFont();
		static ImFont* Get(const std::string& aFontName);
	};
}
