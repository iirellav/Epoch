#pragma once
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui/imgui_internal.h>
#include <imgui/imgui_stdlib.h>
#include "Epoch/ImGui/ImGuiFonts.h"
#include "Epoch/Rendering/Texture.h"

namespace Epoch::UI
{
	class ScopedStyle
	{
	public:
		template<typename T>
		ScopedStyle(ImGuiStyleVar aStyleVar, T aValue) { ImGui::PushStyleVar(aStyleVar, aValue); }
		ScopedStyle(const ScopedStyle&) = delete;
		ScopedStyle& operator=(const ScopedStyle&) = delete;
		~ScopedStyle() { ImGui::PopStyleVar(); }
	};

	class ScopedColor
	{
	public:
		template<typename T>
		ScopedColor(ImGuiCol aColorId, T aColor) { ImGui::PushStyleColor(aColorId, ImColor(aColor).Value); }
		ScopedColor(const ScopedColor&) = delete;
		ScopedColor& operator=(const ScopedColor&) = delete;
		~ScopedColor() { ImGui::PopStyleColor(); }
	};

	class ScopedFont
	{
	public:
		ScopedFont(const std::string& aFontName) { Fonts::PushFont(aFontName); }
		ScopedFont(const ScopedFont&) = delete;
		ScopedFont& operator=(const ScopedFont&) = delete;
		~ScopedFont() { Fonts::PopFont(); }
	};

	class ScopedID
	{
	public:
		template<typename T>
		ScopedID(T aId) { ImGui::PushID(aId); }
		ScopedID(const ScopedID&) = delete;
		ScopedID& operator=(const ScopedID&) = delete;
		~ScopedID() { ImGui::PopID(); }
	};


	class ScopedColorStack
	{
	public:
		ScopedColorStack(const ScopedColorStack&) = delete;
		ScopedColorStack& operator=(const ScopedColorStack&) = delete;

		template <typename ColourType, typename... OtherColours>
		ScopedColorStack(ImGuiCol aFirstColourID, ColourType aFirstColour, OtherColours&& ... aOtherColourPairs)
			: myCount((sizeof... (aOtherColourPairs) / 2) + 1)
		{
			static_assert ((sizeof... (aOtherColourPairs) & 1u) == 0, "ScopedColourStack constructor expects a list of pairs of colour IDs and colours as its arguments");

			PushColor(aFirstColourID, aFirstColour, std::forward<OtherColours>(aOtherColourPairs)...);
		}

		~ScopedColorStack() { ImGui::PopStyleColor(myCount); }

	private:
		int myCount;

		template <typename ColourType, typename... OtherColours>
		void PushColor(ImGuiCol aColourID, ColourType aColor, OtherColours&& ... aOtherColourPairs)
		{
			if constexpr (sizeof... (aOtherColourPairs) == 0)
			{
				ImGui::PushStyleColor(aColourID, ImColor(aColor).Value);
			}
			else
			{
				ImGui::PushStyleColor(aColourID, ImColor(aColor).Value);
				PushColor(std::forward<OtherColours>(aOtherColourPairs)...);
			}
		}
	};

	class ScopedStyleStack
	{
	public:
		ScopedStyleStack(const ScopedStyleStack&) = delete;
		ScopedStyleStack& operator=(const ScopedStyleStack&) = delete;

		template <typename ValueType, typename... OtherStylePairs>
		ScopedStyleStack(ImGuiStyleVar aFirstStyleVar, ValueType aFirstValue, OtherStylePairs&& ... aOtherStylePairs)
			: myCount((sizeof... (aOtherStylePairs) / 2) + 1)
		{
			static_assert ((sizeof... (aOtherStylePairs) & 1u) == 0, "ScopedStyleStack constructor expects a list of pairs of colour IDs and colours as its arguments");

			PushStyle(aFirstStyleVar, aFirstValue, std::forward<OtherStylePairs>(aOtherStylePairs)...);
		}

		~ScopedStyleStack() { ImGui::PopStyleVar(myCount); }

	private:
		int myCount;

		template <typename ValueType, typename... OtherStylePairs>
		void PushStyle(ImGuiStyleVar aStyleVar, ValueType aValue, OtherStylePairs&& ... aOtherStylePairs)
		{
			if constexpr (sizeof... (aOtherStylePairs) == 0)
			{
				ImGui::PushStyleVar(aStyleVar, aValue);
			}
			else
			{
				ImGui::PushStyleVar(aStyleVar, aValue);
				PushStyle(std::forward<OtherStylePairs>(aOtherStylePairs)...);
			}
		}
	};


	inline ImRect GetItemRect()
	{
		return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	}

	inline ImRect RectExpanded(const ImRect& aRect, float aX, float aY)
	{
		ImRect result = aRect;
		result.Min.x -= aX;
		result.Min.y -= aY;
		result.Max.x += aX;
		result.Max.y += aY;
		return result;
	}


	inline void SetTooltip(std::string_view aText, bool aAllowWhenDisabled = true, ImVec2 aPadding = ImVec2(5, 5))
	{
		if (ImGui::IsItemHovered(aAllowWhenDisabled ? ImGuiHoveredFlags_AllowWhenDisabled : 0))
		{
			UI::ScopedStyle tooltipPadding(ImGuiStyleVar_WindowPadding, aPadding);
			//UI::ScopedColor textCol(ImGuiCol_Text, Colors::Theme::textBrighter);
			ImGui::SetTooltip(aText.data());
		}
	}

	
	namespace Draw
	{
		inline void Underline(bool aFullWidth = true, float aOffsetX = 0.0f, float aOffsetY = -1.0f)
		{
			if (aFullWidth)
			{
				if (ImGui::GetCurrentWindow()->DC.CurrentColumns != nullptr)
				{
					ImGui::PushColumnsBackground();
				}
				else if (ImGui::GetCurrentTable() != nullptr)
				{
					ImGui::TablePushBackgroundChannel();
				}
			}
			
			const float width = aFullWidth ? ImGui::GetWindowWidth() : ImGui::GetContentRegionAvail().x;
			const ImVec2 cursor = ImGui::GetCursorScreenPos();
			ImGui::GetWindowDrawList()->AddLine
			(
				ImVec2(cursor.x + aOffsetX, cursor.y + aOffsetY),
				ImVec2(cursor.x + width, cursor.y + aOffsetY),
				IM_COL32(50, 50, 50, 255), 1.0f
			);
			
			if (aFullWidth)
			{
				if (ImGui::GetCurrentWindow()->DC.CurrentColumns != nullptr)
				{
					ImGui::PopColumnsBackground();
				}
				else if (ImGui::GetCurrentTable() != nullptr)
				{
					ImGui::TablePopBackgroundChannel();
				}
			}
		}

		inline void DrawImage(const std::shared_ptr<Texture2D>& aImage,
			ImU32 aTintNormal, ImU32 aTintHovered, ImU32 aTintPressed,
			ImRect aRect)
		{
			auto* drawList = ImGui::GetWindowDrawList();
			if (ImGui::IsItemActive())
			{
				drawList->AddImage((ImTextureID)aImage->GetView(), aRect.Min, aRect.Max, ImVec2(0, 0), ImVec2(1, 1), aTintNormal);
			}
			else if (ImGui::IsItemHovered())
			{
				drawList->AddImage((ImTextureID)aImage->GetView(), aRect.Min, aRect.Max, ImVec2(0, 0), ImVec2(1, 1), aTintHovered);
			}
			else
			{
				drawList->AddImage((ImTextureID)aImage->GetView(), aRect.Min, aRect.Max, ImVec2(0, 0), ImVec2(1, 1), aTintPressed);
			}
		};
	}

	//=========================================================================================
	/// Custom IMGui controls

	inline int FormatString(char* buf, size_t buf_size, const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
#ifdef IMGUI_USE_STB_SPRINTF
		int w = stbsp_vsnprintf(buf, (int)buf_size, fmt, args);
#else
		int w = vsnprintf(buf, buf_size, fmt, args);
#endif
		va_end(args);
		if (buf == NULL)
			return w;
		if (w == -1 || w >= (int)buf_size)
			w = (int)buf_size - 1;
		buf[w] = 0;
		return w;
	}

	inline bool DragScalar(const char* label, ImGuiDataType data_type, void* p_data, float v_speed, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		const float w = ImGui::CalcItemWidth();

		const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
		const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f));
		const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

		const bool temp_input_allowed = (flags & ImGuiSliderFlags_NoInput) == 0;
		ImGui::ItemSize(total_bb, style.FramePadding.y);
		if (!ImGui::ItemAdd(total_bb, id, &frame_bb, temp_input_allowed ? ImGuiItemFlags_Inputable : 0))
			return false;

		// Default format string when passing NULL
		if (format == NULL)
			format = ImGui::DataTypeGetInfo(data_type)->PrintFmt;

		const bool hovered = ImGui::ItemHoverable(frame_bb, id, g.LastItemData.InFlags);
		bool temp_input_is_active = temp_input_allowed && ImGui::TempInputIsActive(id);
		if (!temp_input_is_active)
		{
			// Tabbing or CTRL-clicking on Drag turns it into an InputText
			const bool input_requested_by_tabbing = temp_input_allowed && (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_FocusedByTabbing) != 0;
			const bool clicked = hovered && ImGui::IsMouseClicked(0, id);
			const bool double_clicked = (hovered && g.IO.MouseClickedCount[0] == 2 && ImGui::TestKeyOwner(ImGuiKey_MouseLeft, id));
			const bool make_active = (input_requested_by_tabbing || clicked || double_clicked || g.NavActivateId == id);
			if (make_active && (clicked || double_clicked))
				ImGui::SetKeyOwner(ImGuiKey_MouseLeft, id);
			if (make_active && temp_input_allowed)
				if (input_requested_by_tabbing || (clicked && g.IO.KeyCtrl) || double_clicked || (g.NavActivateId == id && (g.NavActivateFlags & ImGuiActivateFlags_PreferInput)))
					temp_input_is_active = true;

			// (Optional) simple click (without moving) turns Drag into an InputText
			if (g.IO.ConfigDragClickToInputText && temp_input_allowed && !temp_input_is_active)
				if (g.ActiveId == id && hovered && g.IO.MouseReleased[0] && !ImGui::IsMouseDragPastThreshold(0, g.IO.MouseDragThreshold * 0.5f))
				{
					g.NavActivateId = id;
					g.NavActivateFlags = ImGuiActivateFlags_PreferInput;
					temp_input_is_active = true;
				}

			if (make_active && !temp_input_is_active)
			{
				ImGui::SetActiveID(id, window);
				ImGui::SetFocusID(id, window);
				ImGui::FocusWindow(window);
				g.ActiveIdUsingNavDirMask = (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
			}
		}

		if (temp_input_is_active)
		{
			// Only clamp CTRL+Click input when ImGuiSliderFlags_AlwaysClamp is set
			const bool is_clamp_input = (flags & ImGuiSliderFlags_AlwaysClamp) != 0 && (p_min == NULL || p_max == NULL || ImGui::DataTypeCompare(data_type, p_min, p_max) < 0);
			return ImGui::TempInputScalar(frame_bb, id, label, data_type, p_data, format, is_clamp_input ? p_min : NULL, is_clamp_input ? p_max : NULL);
		}

		// Draw frame
		const ImU32 frame_col = ImGui::GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
		ImGui::RenderNavHighlight(frame_bb, id);
		ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, style.FrameRounding);

		// Drag behavior
		const bool value_changed = ImGui::DragBehavior(id, data_type, p_data, v_speed, p_min, p_max, format, flags);
		if (value_changed)
			ImGui::MarkItemEdited(id);

		const bool mixed_value = (g.CurrentItemFlags & ImGuiItemFlags_MixedValue) != 0;

		// Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
		char value_buf[64];
		const char* value_buf_end = value_buf + (mixed_value ? FormatString(value_buf, IM_ARRAYSIZE(value_buf), "---") : ImGui::DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format));
		if (g.LogEnabled)
			ImGui::LogSetNextTextDecoration("{", "}");
		ImGui::RenderTextClipped(frame_bb.Min, frame_bb.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f, 0.5f));

		if (label_size.x > 0.0f)
			ImGui::RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

		IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | (temp_input_allowed ? ImGuiItemStatusFlags_Inputable : 0));
		return value_changed;
	}
	
	inline bool DragFloat(const char* label, float* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
	{
		return DragScalar(label, ImGuiDataType_Float, v, v_speed, &v_min, &v_max, format, flags);
	}
}
