#include "epch.h"
#include "ImGuiWidgets.h"
#include "ImGuiUtilities.h"
#include "ImGui.h"
#include "UICore.h"
#include "Epoch/Scene/Scene.h"
#include "Epoch/Project/Project.h"

namespace Epoch::UI::Widgets
{
	void GradientBar(const CU::Gradient* aGradient, CU::Vector2f aBarPos, float aMaxWidth, float aHeight)
	{
		EPOCH_PROFILE_FUNC();

		aMaxWidth = CU::Math::Clamp(aMaxWidth, 10.0f, ImGui::GetContentRegionAvail().x);

		//float prevX = aBarPos.x;
		float barBottom = aBarPos.y + aHeight;
		ImDrawList* draw_list = ImGui::GetWindowDrawList();


		ImU32 lightCol = ImGui::ColorConvertFloat4ToU32(ImVec4(0.75f, 0.75f, 0.75f, 1.0f));
		ImU32 darkCol = ImGui::ColorConvertFloat4ToU32(ImVec4(0.6f, 0.6f, 0.6f, 1.0f));

		float backgroundTileSize = aHeight / 2;
		int backgroundTileCount = (int)(aMaxWidth / backgroundTileSize);

		float from = 0.0f;
		float to = 0.0f;
		for (int i = 0; i < backgroundTileCount; i++)
		{
			from = aBarPos.x + i * backgroundTileSize;
			to = aBarPos.x + (i + 1) * backgroundTileSize;

			if (i % 2)
			{
				draw_list->AddRectFilled(ImVec2(from, aBarPos.y), ImVec2(to, aBarPos.y + backgroundTileSize), lightCol);
				draw_list->AddRectFilled(ImVec2(from, aBarPos.y + backgroundTileSize), ImVec2(to, aBarPos.y + backgroundTileSize * 2), darkCol);
			}
			else
			{
				draw_list->AddRectFilled(ImVec2(from, aBarPos.y), ImVec2(to, aBarPos.y + backgroundTileSize), darkCol);
				draw_list->AddRectFilled(ImVec2(from, aBarPos.y + backgroundTileSize), ImVec2(to, aBarPos.y + backgroundTileSize * 2), lightCol);
			}
		}

		from = aBarPos.x + backgroundTileCount * backgroundTileSize;
		to = aBarPos.x + aMaxWidth;

		if (backgroundTileCount % 2)
		{
			draw_list->AddRectFilled(ImVec2(from, aBarPos.y), ImVec2(to, aBarPos.y + backgroundTileSize), lightCol);
			draw_list->AddRectFilled(ImVec2(from, aBarPos.y + backgroundTileSize), ImVec2(to, aBarPos.y + backgroundTileSize * 2), darkCol);
		}
		else
		{
			draw_list->AddRectFilled(ImVec2(from, aBarPos.y), ImVec2(to, aBarPos.y + backgroundTileSize), darkCol);
			draw_list->AddRectFilled(ImVec2(from, aBarPos.y + backgroundTileSize), ImVec2(to, aBarPos.y + backgroundTileSize * 2), lightCol);
		}

		from = 0.0f;
		to = 0.0f;
		int step = 0;
		float stepValue = aMaxWidth / 256;
		for (const CU::Color& color : aGradient->myCachedValues)
		{
			from = aBarPos.x + step * stepValue;
			to = aBarPos.x + (step + 1) * stepValue;

			draw_list->AddRectFilled(ImVec2(from, aBarPos.y), ImVec2(to, barBottom), ImGui::ColorConvertFloat4ToU32(ImVec4(color.r, color.g, color.b, color.a)));

			step++;
		}

		ImGui::SetCursorScreenPos(ImVec2(aBarPos.x, aBarPos.y + aHeight + 4.0f));
	}

	void CubicBezier(std::array<CU::Vector2f, 4> aPoints, CU::Vector2f aBarPos, float aMaxWidth, float aHeight, bool aDrawLines, float aThickness)
	{
		aMaxWidth = CU::Math::Clamp(aMaxWidth, 10.0f, ImGui::GetContentRegionAvail().x);

		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		
		{
			const ImU32 col = ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Button]);
			draw_list->AddRectFilled({ aBarPos.x, aBarPos.y }, { aBarPos.x + aMaxWidth, aBarPos.y + aHeight }, col);
		}

		if (aDrawLines)
		{
			const ImU32 col = ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
			const float yIncrement = aHeight / 10.0f;
			const float xIncrement = aMaxWidth / 10.0f;

			for (size_t i = 1; i < 10; i++)
			{
				const float yPos = aBarPos.y + yIncrement * i;
				draw_list->AddLine({ aBarPos.x, yPos }, { aBarPos.x + aMaxWidth, yPos }, col);

				const float xPos = aBarPos.x + xIncrement * i;
				draw_list->AddLine({ xPos, aBarPos.y }, { xPos, aBarPos.y + aHeight }, col);
			}
		}

		const ImVec2 ip1 = ImVec2(aBarPos.x + aMaxWidth * aPoints[0].x, aBarPos.y + aHeight * (1.0f - aPoints[0].y));
		const ImVec2 ip2 = ImVec2(aBarPos.x + aMaxWidth * aPoints[1].x, aBarPos.y + aHeight * (1.0f - aPoints[1].y));
		const ImVec2 ip3 = ImVec2(aBarPos.x + aMaxWidth * aPoints[2].x, aBarPos.y + aHeight * (1.0f - aPoints[2].y));
		const ImVec2 ip4 = ImVec2(aBarPos.x + aMaxWidth * aPoints[3].x, aBarPos.y + aHeight * (1.0f - aPoints[3].y));

		draw_list->AddBezierCubic(ip1, ip2, ip3, ip4, Colors::Theme::blue, 2, 32);

		if (aDrawLines)
		{
			const ImU32 col = ImGui::ColorConvertFloat4ToU32({ 0.5f, 0.5f, 0.5f, 1.0f });

			draw_list->AddCircleFilled(ip2, 8.0f, col);
			draw_list->AddCircleFilled(ip3, 8.0f, col);

			draw_list->AddLine(ip1, ip2, col, 2.0f);
			draw_list->AddLine(ip3, ip4, col, 2.0f);
		}

		ImGui::SetCursorScreenPos(ImVec2(aBarPos.x, aBarPos.y + aHeight + 4.0f));
	}

	void Spinner(const char* aLabel, float aRadius, float aThickness, const uint32_t aColor)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
		{
			return;
		}

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(aLabel);

		ImVec2 pos = window->DC.CursorPos;
		ImVec2 size((aRadius) * 2, (aRadius + style.FramePadding.y) * 2);

		const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
		ImGui::ItemSize(bb, style.FramePadding.y);
		if (!ImGui::ItemAdd(bb, id))
		{
			return;
		}

		window->DrawList->PathClear();

		float num_segments = 30.0f;
		float start = abs(ImSin((float)g.Time * 1.8f) * (num_segments - 5.0f));

		const float a_min = IM_PI * 2.0f * (start) / num_segments;
		const float a_max = IM_PI * 2.0f * (num_segments - 3) / num_segments;

		const ImVec2 center = ImVec2(pos.x + aRadius, pos.y + aRadius + style.FramePadding.y);

		for (int i = 0; i < num_segments; i++)
		{
			const float a = a_min + ((float)i / num_segments) * (a_max - a_min);
			window->DrawList->PathLineTo(ImVec2(center.x + ImCos(a + (float)g.Time * 8.0f) * aRadius, center.y + ImSin(a + (float)g.Time * 8.0f) * aRadius));
		}

		window->DrawList->PathStroke(aColor, ImDrawFlags_None, aThickness);
	}

	void BufferingBar(const char* aLabel, float aValue, CU::Vector2f aSize, uint32_t aBgCol, uint32_t aFgCol)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
		{
			return;
		}

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(aLabel);

		ImVec2 pos = window->DC.CursorPos;
		ImVec2 size = { aSize.x, aSize.y };
		size.x -= style.FramePadding.x * 2.0f;

		const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
		ImGui::ItemSize(bb, style.FramePadding.y);
		if (!ImGui::ItemAdd(bb, id))
		{
			return;
		}

		const float circleStart = size.x * 0.7f;
		const float circleEnd = size.x;
		const float circleWidth = circleEnd - circleStart;

		window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart, bb.Max.y), aBgCol);
		window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart * aValue, bb.Max.y), aFgCol);

		const float t = (float)g.Time;
		const float r = size.y / 2;
		const float speed = 1.5f;

		const float a = speed * 0;
		const float b = speed * 0.333f;
		const float c = speed * 0.666f;

		const float o1 = (circleWidth + r) * (t + a - speed * (int)((t + a) / speed)) / speed;
		const float o2 = (circleWidth + r) * (t + b - speed * (int)((t + b) / speed)) / speed;
		const float o3 = (circleWidth + r) * (t + c - speed * (int)((t + c) / speed)) / speed;

		window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o1, bb.Min.y + r), r, aBgCol);
		window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o2, bb.Min.y + r), r, aBgCol);
		window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o3, bb.Min.y + r), r, aBgCol);
	}

	bool AssetSearchPopup(const char* aPopupID, AssetType aAssetType, AssetHandle& outSelected, bool* outClear)
	{
		EPOCH_PROFILE_FUNC();

		bool modified = false;

		UI::ScopedColor popupBG(ImGuiCol_PopupBg, IM_COL32(26, 26, 26, 255));

		const auto& assetRegistry = Project::GetEditorAssetManager()->GetAssetRegistry();
		AssetHandle current = outSelected;

		ImGui::SetNextWindowSize({ 300.0f, 0.0f });

		static bool grabFocus = true;
		
		if (ImGui::BeginPopup(aPopupID, ImGuiWindowFlags_NoResize))
		{
			static std::string searchString;

			if (ImGui::GetCurrentWindow()->Appearing)
			{
				grabFocus = true;
				searchString.clear();
			}

			UI::ShiftCursor(-3.0f, -2.0f);

			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - ImGui::GetCursorPosX() * 2.0f);
			ImGui::InputTextWithHint("##AssetSearch", "Search...", &searchString);

			if (grabFocus)
			{
				if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)
					&& !ImGui::IsAnyItemActive()
					&& !ImGui::IsMouseClicked(0))
				{
					ImGui::SetKeyboardFocusHere(-1);
				}

				if (ImGui::IsItemFocused())
				{
					grabFocus = false;
				}
			}
			
			const bool searching = !searchString.empty();
			
			// Clear property button
			if (outClear != nullptr)
			{
				//UI::ScopedStyle border(ImGuiStyleVar_FrameBorderSize, 0.0f);

				ImGui::SetCursorPosX(0);

				ImGui::PushItemFlag(ImGuiItemFlags_NoNav, searching);

				if (ImGui::Button("CLEAR", { ImGui::GetWindowWidth()/*ImGui::GetContentRegionAvail().x*/, 0.0f}))
				{
					*outClear = true;
					modified = true;
				}

				ImGui::PopItemFlag();
			}

			// List of assets
			{
				ImGuiID listID = ImGui::GetID("##SearchListBox");
				if (ImGui::BeginListBox("##SearchListBox", ImVec2(-FLT_MIN, 0.0f)))
				{
					bool forwardFocus = false;

					ImGuiContext& g = *GImGui;
					if (g.NavJustMovedToId != 0)
					{
						if (g.NavJustMovedToId == listID)
						{
							forwardFocus = true;
							// ActivateItem moves keyboard navigation focus inside of the window
							ImGui::ActivateItemByID(listID);
							ImGui::SetKeyboardFocusHere(1);
						}
					}

					for (const auto& [path, metadata] : assetRegistry)
					{
						if (metadata.type != aAssetType) continue;

						const std::string assetName = metadata.isMemoryAsset ? metadata.filePath.string() : metadata.filePath.stem().string();

						if (!UI::IsMatchingSearch(assetName, searchString)) continue;

						ImGui::PushID(metadata.filePath.string().c_str());

						bool isSelected = (current == metadata.handle);
						if (ImGui::Selectable(assetName.c_str(), isSelected))
						{
							current = metadata.handle;
							outSelected = metadata.handle;
							modified = true;
						}

						ImGui::PopID();

						if (forwardFocus)
						{
							forwardFocus = false;
						}
						else if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}

					ImGui::EndListBox();
				}
			}

			if (modified)
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		return modified;
	}

	bool EntitySearchPopup(const char* aPopupID, std::shared_ptr<Scene> aScene, UUID& outSelected, bool* outClear)
	{
		EPOCH_PROFILE_FUNC();

		bool modified = false;

		UI::ScopedColor popupBG(ImGuiCol_PopupBg, IM_COL32(26, 26, 26, 255));

		auto entities = aScene->GetAllEntitiesWith<IDComponent, NameComponent>();
		UUID current = outSelected;

		ImGui::SetNextWindowSize({ 300.0f, 0.0f });

		static bool grabFocus = true;
		
		if (ImGui::BeginPopup(aPopupID, ImGuiWindowFlags_NoResize))
		{
			static std::string searchString;

			if (ImGui::GetCurrentWindow()->Appearing)
			{
				grabFocus = true;
				searchString.clear();
			}

			UI::ShiftCursor(-3.0f, -2.0f);

			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - ImGui::GetCursorPosX() * 2.0f);
			ImGui::InputTextWithHint("##AssetSearch", "Search...", &searchString);

			if (grabFocus)
			{
				if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)
					&& !ImGui::IsAnyItemActive()
					&& !ImGui::IsMouseClicked(0))
				{
					ImGui::SetKeyboardFocusHere(-1);
				}

				if (ImGui::IsItemFocused())
				{
					grabFocus = false;
				}
			}
			
			const bool searching = !searchString.empty();
			
			// Clear property button
			if (outClear != nullptr)
			{
				//UI::ScopedStyle border(ImGuiStyleVar_FrameBorderSize, 0.0f);

				ImGui::SetCursorPosX(0);

				ImGui::PushItemFlag(ImGuiItemFlags_NoNav, searching);

				if (ImGui::Button("CLEAR", { ImGui::GetWindowWidth()/*ImGui::GetContentRegionAvail().x*/, 0.0f}))
				{
					*outClear = true;
					modified = true;
				}

				ImGui::PopItemFlag();
			}

			// List of assets
			{
				ImGuiID listID = ImGui::GetID("##SearchListBox");
				if (ImGui::BeginListBox("##SearchListBox", ImVec2(-FLT_MIN, 0.0f)))
				{
					bool forwardFocus = false;

					ImGuiContext& g = *GImGui;
					if (g.NavJustMovedToId != 0)
					{
						if (g.NavJustMovedToId == listID)
						{
							forwardFocus = true;
							// ActivateItem moves keyboard navigation focuse inside of the window
							ImGui::ActivateItemByID(listID);
							ImGui::SetKeyboardFocusHere(1);
						}
					}

					for (auto enttID : entities)
					{
						const auto& idComponent = entities.get<IDComponent>(enttID);
						const auto& nameComponent = entities.get<NameComponent>(enttID);

						if (!searchString.empty() && !UI::IsMatchingSearch(nameComponent.name, searchString))
						{
							continue;
						}

						bool is_selected = current == idComponent.id;
						if (ImGui::Selectable(nameComponent.name.c_str(), is_selected))
						{
							current = outSelected = idComponent.id;
							modified = true;
						}

						if (forwardFocus)
						{
							forwardFocus = false;
						}
						else if (is_selected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}

					ImGui::EndListBox();
				}
			}

			if (modified)
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		return modified;
	}
}
