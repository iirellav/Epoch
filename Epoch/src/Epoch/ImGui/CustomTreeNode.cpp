#include "epch.h"
#include "CustomTreeNode.h"

#include "Colors.h"
#include "ImGuiUtilities.h"
#include "UICore.h"

namespace ImGui
{
	bool TreeNodeWithIcon(std::shared_ptr<Epoch::Texture2D> aIcon, ImGuiID aId, ImGuiTreeNodeFlags aFlags, const char* aLabel, const char* aLabelEnd, ImColor aIconTint)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
		{
			return false;
		}

		ImGuiContext& g = *GImGui;
		ImGuiLastItemData& lastItem = g.LastItemData;
		const ImGuiStyle& style = g.Style;
		const bool display_frame = (aFlags & ImGuiTreeNodeFlags_Framed) != 0;
		const ImVec2 padding = (display_frame || (aFlags & ImGuiTreeNodeFlags_FramePadding)) ? style.FramePadding : ImVec2(style.FramePadding.x, ImMin(window->DC.CurrLineTextBaseOffset, style.FramePadding.y));

		if (!aLabelEnd)
		{
			aLabelEnd = FindRenderedTextEnd(aLabel);
		}
		const ImVec2 label_size = CalcTextSize(aLabel, aLabelEnd, false);

		// We vertically grow up to current line height up the typical widget height.
		const float frame_height = ImMax(ImMin(window->DC.CurrLineSize.y, g.FontSize + style.FramePadding.y * 2), label_size.y + padding.y * 2);
		ImRect frame_bb;
		frame_bb.Min.x = (aFlags & ImGuiTreeNodeFlags_SpanFullWidth) ? window->WorkRect.Min.x : window->DC.CursorPos.x;
		frame_bb.Min.y = window->DC.CursorPos.y;
		frame_bb.Max.x = window->WorkRect.Max.x;
		frame_bb.Max.y = window->DC.CursorPos.y + frame_height;
		if (display_frame)
		{
			// Framed header expand a little outside the default padding, to the edge of InnerClipRect
			// (FIXME: May remove this at some point and make InnerClipRect align with WindowPadding.x instead of WindowPadding.x*0.5f)
			frame_bb.Min.x -= IM_FLOOR(window->WindowPadding.x * 0.5f - 1.0f);
			frame_bb.Max.x += IM_FLOOR(window->WindowPadding.x * 0.5f);
		}

		const float text_offset_x = g.FontSize + (display_frame ? padding.x * 3 : padding.x * 2);           // Collapser arrow width + Spacing
		const float text_offset_y = ImMax(padding.y, window->DC.CurrLineTextBaseOffset);                    // Latch before ItemSize changes it
		const float text_width = g.FontSize + (label_size.x > 0.0f ? label_size.x + padding.x * 2 : 0.0f);  // Include collapser
		ImVec2 text_pos(window->DC.CursorPos.x + text_offset_x, window->DC.CursorPos.y + text_offset_y);
		ItemSize(ImVec2(text_width, frame_height), padding.y);

		// For regular tree nodes, we arbitrary allow to click past 2 worth of ItemSpacing
		ImRect interact_bb = frame_bb;
		if (!display_frame && (aFlags & (ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth)) == 0)
		{
			interact_bb.Max.x = frame_bb.Min.x + text_width + style.ItemSpacing.x * 2.0f;
		}

		// Store a flag for the current depth to tell if we will allow closing this node when navigating one of its child.
		// For this purpose we essentially compare if g.NavIdIsAlive went from 0 to 1 between TreeNode() and TreePop().
		// This is currently only support 32 level deep and we are fine with (1 << Depth) overflowing into a zero.
		const bool is_leaf = (aFlags & ImGuiTreeNodeFlags_Leaf) != 0;
		bool is_open = TreeNodeBehaviorIsOpen(aId, aFlags);
		if (is_open && !g.NavIdIsAlive && (aFlags & ImGuiTreeNodeFlags_NavLeftJumpsBackHere) && !(aFlags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
		{
			window->DC.TreeJumpToParentOnPopMask |= (1 << window->DC.TreeDepth);
		}

		bool item_add = ItemAdd(interact_bb, aId);
		lastItem.StatusFlags |= ImGuiItemStatusFlags_HasDisplayRect;
		lastItem.DisplayRect = frame_bb;

		if (!item_add)
		{
			if (is_open && !(aFlags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
			{
				TreePushOverrideID(aId);
			}
			IMGUI_TEST_ENGINE_ITEM_INFO(lastItem.ID, aLabel, lastItem.StatusFlags | (is_leaf ? 0 : ImGuiItemStatusFlags_Openable) | (is_open ? ImGuiItemStatusFlags_Opened : 0));
			return is_open;
		}

		ImGuiButtonFlags button_flags = ImGuiTreeNodeFlags_None;
		if (aFlags & ImGuiTreeNodeFlags_AllowItemOverlap)
		{
			button_flags |= ImGuiButtonFlags_AllowOverlap;
		}
		if (!is_leaf)
		{
			button_flags |= ImGuiButtonFlags_PressedOnDragDropHold;
		}

		// We allow clicking on the arrow section with keyboard modifiers held, in order to easily
		// allow browsing a tree while preserving selection with code implementing multi-selection patterns.
		// When clicking on the rest of the tree node we always disallow keyboard modifiers.
		const float arrow_hit_x1 = (text_pos.x - text_offset_x) - style.TouchExtraPadding.x;
		const float arrow_hit_x2 = (text_pos.x - text_offset_x) + (g.FontSize + padding.x * 2.0f) + style.TouchExtraPadding.x;
		const bool is_mouse_x_over_arrow = (g.IO.MousePos.x >= arrow_hit_x1 && g.IO.MousePos.x < arrow_hit_x2);
		if (window != g.HoveredWindow || !is_mouse_x_over_arrow)
		{
			button_flags |= ImGuiButtonFlags_NoKeyModifiers;
		}

		// Open behaviors can be altered with the _OpenOnArrow and _OnOnDoubleClick flags.
		// Some alteration have subtle effects (e.g. toggle on MouseUp vs MouseDown events) due to requirements for multi-selection and drag and drop support.
		// - Single-click on label = Toggle on MouseUp (default, when _OpenOnArrow=0)
		// - Single-click on arrow = Toggle on MouseDown (when _OpenOnArrow=0)
		// - Single-click on arrow = Toggle on MouseDown (when _OpenOnArrow=1)
		// - Double-click on label = Toggle on MouseDoubleClick (when _OpenOnDoubleClick=1)
		// - Double-click on arrow = Toggle on MouseDoubleClick (when _OpenOnDoubleClick=1 and _OpenOnArrow=0)
		// It is rather standard that arrow click react on Down rather than Up.
		// We set ImGuiButtonFlags_PressedOnClickRelease on OpenOnDoubleClick because we want the item to be active on the initial MouseDown in order for drag and drop to work.
		if (is_mouse_x_over_arrow)
		{
			button_flags |= ImGuiButtonFlags_PressedOnClick;
		}
		else if (aFlags & ImGuiTreeNodeFlags_OpenOnDoubleClick)
		{
			button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick;
		}
		else
		{
			button_flags |= ImGuiButtonFlags_PressedOnClickRelease;
		}

		bool selected = (aFlags & ImGuiTreeNodeFlags_Selected) != 0;
		const bool was_selected = selected;

		bool hovered, held;
		bool pressed = ButtonBehavior(interact_bb, aId, &hovered, &held, button_flags);
		bool toggled = false;
		if (!is_leaf)
		{
			if (pressed && g.DragDropHoldJustPressedId != aId)
			{
				if ((aFlags & (ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick)) == 0 || (g.NavActivateId == aId))
					toggled = true;
				if (aFlags & ImGuiTreeNodeFlags_OpenOnArrow)
					toggled |= is_mouse_x_over_arrow && !g.NavDisableMouseHover; // Lightweight equivalent of IsMouseHoveringRect() since ButtonBehavior() already did the job
				if ((aFlags & ImGuiTreeNodeFlags_OpenOnDoubleClick) && g.IO.MouseDoubleClicked[0])
					toggled = true;
			}
			else if (pressed && g.DragDropHoldJustPressedId == aId)
			{
				IM_ASSERT(button_flags & ImGuiButtonFlags_PressedOnDragDropHold);
				if (!is_open) // When using Drag and Drop "hold to open" we keep the node highlighted after opening, but never close it again.
				{
					toggled = true;
				}
			}

			if (g.NavId == aId && g.NavMoveDir == ImGuiDir_Left && is_open)
			{
				toggled = true;
				NavMoveRequestCancel();
			}
			if (g.NavId == aId && g.NavMoveDir == ImGuiDir_Right && !is_open) // If there's something upcoming on the line we may want to give it the priority?
			{
				toggled = true;
				NavMoveRequestCancel();
			}

			if (toggled)
			{
				is_open = !is_open;
				window->DC.StateStorage->SetInt(aId, is_open);
				lastItem.StatusFlags |= ImGuiItemStatusFlags_ToggledOpen;
			}
		}
		if (aFlags & ImGuiTreeNodeFlags_AllowItemOverlap)
		{
			SetItemAllowOverlap();
		}

		// In this branch, TreeNodeBehavior() cannot toggle the selection so this will never trigger.
		if (selected != was_selected) //-V547
		{
			lastItem.StatusFlags |= ImGuiItemStatusFlags_ToggledSelection;
		}

		// Render

		//const ImU32 arrow_col = selected ? Colors::Theme::backgroundDark : Colors::Theme::muted;
		const ImU32 arrow_col = GetColorU32(ImGuiCol_Text);

		ImGuiNavHighlightFlags nav_highlight_flags = ImGuiNavHighlightFlags_TypeThin;
		if (display_frame)
		{
			// Framed type
			const ImU32 bg_col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : (hovered && !selected && !held && !pressed && !toggled) ? ImGuiCol_HeaderHovered : ImGuiCol_Header);

			RenderFrame(frame_bb.Min, frame_bb.Max, bg_col, true, style.FrameRounding);
			RenderNavHighlight(frame_bb, aId, nav_highlight_flags);
			if (aFlags & ImGuiTreeNodeFlags_Bullet)
			{
				RenderBullet(window->DrawList, ImVec2(text_pos.x - text_offset_x * 0.60f, text_pos.y + g.FontSize * 0.5f), arrow_col);
			}
			else if (!is_leaf)
			{
				RenderArrow(window->DrawList, ImVec2(text_pos.x - text_offset_x + padding.x, text_pos.y), arrow_col, is_open ? ImGuiDir_Down : ImGuiDir_Right, 1.0f);
			}
			else // Leaf without bullet, left-adjusted text
			{
				text_pos.x -= text_offset_x;
			}
			if (aFlags & ImGuiTreeNodeFlags_ClipLabelForTrailingButton)
			{
				frame_bb.Max.x -= g.FontSize + style.FramePadding.x;
			}

			//! Draw icon
			if (aIcon)
			{
				// Store item data
				auto itemId = lastItem.ID;
				auto itemFlags = lastItem.InFlags;
				auto itemStatusFlags = lastItem.StatusFlags;
				auto itemRect = lastItem.Rect;

				// Draw icon image which messes up last item data
				const float pad = 3.0f;
				//const float arrowWidth = 20.0f + 1.0f;
				const float arrowWidth = 20.0f + 2.0f;
				auto cursorPos = ImGui::GetCursorPos();
				Epoch::UI::ShiftCursorY(-frame_height + pad);
				Epoch::UI::ShiftCursorX(arrowWidth);
				Epoch::UI::Draw::Image(aIcon, { frame_height - pad * 2.0f, frame_height - pad * 2.0f });

				// Restore item data
				ImGui::SetLastItemData(itemId, itemFlags, itemStatusFlags, itemRect);

				text_pos.x += frame_height/* + 2.0f*/;
			}

			text_pos.y -= 1.0f;



			if (g.LogEnabled)
			{
				// NB: '##' is normally used to hide text (as a library-wide feature), so we need to specify the text range to make sure the ## aren't stripped out here.
				const char log_prefix[] = "\n##";
				const char log_suffix[] = "##";
				LogRenderedText(&text_pos, log_prefix, log_prefix + 3);
				RenderTextClipped(text_pos, frame_bb.Max, aLabel, aLabelEnd, &label_size);
				LogRenderedText(&text_pos, log_suffix, log_suffix + 2);
			}
			else
			{
				RenderTextClipped(text_pos, frame_bb.Max, aLabel, aLabelEnd, &label_size);
			}
		}
		else
		{
			// Unframed typed for tree nodes
			if (hovered || selected)
			{
				//if (held && hovered) HZ_CORE_WARN("held && hovered");
				//if(hovered && !selected && !held && !pressed && !toggled) HZ_CORE_WARN("hovered && !selected && !held");
				//else if(!selected) HZ_CORE_WARN("ImGuiCol_Header");

				const ImU32 bg_col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : (hovered && !selected && !held && !pressed && !toggled) ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
				RenderFrame(frame_bb.Min, frame_bb.Max, bg_col, false);
				RenderNavHighlight(frame_bb, aId, nav_highlight_flags);
			}
			if (aFlags & ImGuiTreeNodeFlags_Bullet)
			{
				RenderBullet(window->DrawList, ImVec2(text_pos.x - text_offset_x * 0.5f, text_pos.y + g.FontSize * 0.5f), arrow_col);
			}
			else if (!is_leaf)
			{
				RenderArrow(window->DrawList, ImVec2(text_pos.x - text_offset_x + padding.x, text_pos.y + g.FontSize * 0.15f), arrow_col, is_open ? ImGuiDir_Down : ImGuiDir_Right, 0.70f);
			}

			//! Draw icon
			if (aIcon)
			{
				// Store item data
				auto itemId = lastItem.ID;
				auto itemFlags = lastItem.InFlags;
				auto itemStatusFlags = lastItem.StatusFlags;
				auto itemRect = lastItem.Rect;

				// Draw icon image which messes up last item data
				const float pad = 3.0f;
				//const float arrowWidth = 20.0f + 1.0f;
				const float arrowWidth = 20.0f + 2.0f;
				auto cursorPos = ImGui::GetCursorPos();
				Epoch::UI::ShiftCursorY(-frame_height + pad);
				Epoch::UI::ShiftCursorX(arrowWidth);
				Epoch::UI::Draw::Image(aIcon, { frame_height - pad * 2.0f, frame_height - pad * 2.0f });

				// Restore item data
				ImGui::SetLastItemData(itemId, itemFlags, itemStatusFlags, itemRect);

				text_pos.x += frame_height/* + 2.0f*/;
			}

			text_pos.y -= 1.0f;


			if (g.LogEnabled)
			{
				LogRenderedText(&text_pos, ">");
			}
			RenderText(text_pos, aLabel, aLabelEnd, false);
		}

		if (is_open && !(aFlags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
		{
			TreePushOverrideID(aId);
		}
		IMGUI_TEST_ENGINE_ITEM_INFO(aId, aLabel, window->DC.ItemFlags | (is_leaf ? 0 : ImGuiItemStatusFlags_Openable) | (is_open ? ImGuiItemStatusFlags_Opened : 0));
		return is_open;
	}

	bool TreeNodeWithIcon(std::shared_ptr<Epoch::Texture2D> aIcon, const char* aLabel, ImGuiTreeNodeFlags aFlags, ImColor aIconTint)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
		{
			return false;
		}

		return TreeNodeWithIcon(aIcon, window->GetID(aLabel), aFlags, aLabel, NULL, aIconTint);
	}
}
