#pragma once
#include <imgui/imgui.h>

namespace Colors
{
	namespace Theme
	{
		constexpr auto blue						= IM_COL32(82, 155, 232, 255);
		constexpr auto disabledBlue				= IM_COL32(45, 85, 128, 255);
		constexpr auto invalid					= IM_COL32(255, 50, 30, 255);
		constexpr auto disabledInvalid			= IM_COL32(140, 27, 27, 255);
		constexpr auto disabled					= IM_COL32(140, 140, 140, 255);

		constexpr auto debug					= IM_COL32(66, 66, 66, 255);
		constexpr auto info						= IM_COL32(79, 192, 255, 255);
		constexpr auto warning					= IM_COL32(255, 227, 15, 255);
		constexpr auto error					= IM_COL32(255, 79, 79, 255);

		constexpr auto dragBoxFill				= IM_COL32(38, 147, 255, 50);
		constexpr auto dragBoxFrame				= IM_COL32(38, 147, 255, 150);
	}
}
