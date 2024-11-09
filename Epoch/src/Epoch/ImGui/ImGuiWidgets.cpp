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

		aMaxWidth = CU::Math::Clamp(aMaxWidth, 10.0f, (float)INT32_MAX);

		//float prevX = aBarPos.x;
		float barBottom = aBarPos.y + aHeight;
		ImDrawList* draw_list = ImGui::GetWindowDrawList();


		ImU32 lightCol = ImGui::ColorConvertFloat4ToU32(ImVec4(0.75f, 0.75f, 0.75f, 1.0f));
		ImU32 darkCol = ImGui::ColorConvertFloat4ToU32(ImVec4(0.6f, 0.6f, 0.6f, 1.0f));

		float backgroundTileSize = aHeight / 3;
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
				draw_list->AddRectFilled(ImVec2(from, aBarPos.y + backgroundTileSize * 2), ImVec2(to, aBarPos.y + backgroundTileSize * 3), lightCol);
			}
			else
			{
				draw_list->AddRectFilled(ImVec2(from, aBarPos.y), ImVec2(to, aBarPos.y + backgroundTileSize), darkCol);
				draw_list->AddRectFilled(ImVec2(from, aBarPos.y + backgroundTileSize), ImVec2(to, aBarPos.y + backgroundTileSize * 2), lightCol);
				draw_list->AddRectFilled(ImVec2(from, aBarPos.y + backgroundTileSize * 2), ImVec2(to, aBarPos.y + backgroundTileSize * 3), darkCol);
			}
		}

		from = aBarPos.x + backgroundTileCount * backgroundTileSize;
		to = aBarPos.x + aMaxWidth;

		if (backgroundTileCount % 2)
		{
			draw_list->AddRectFilled(ImVec2(from, aBarPos.y), ImVec2(to, aBarPos.y + backgroundTileSize), lightCol);
			draw_list->AddRectFilled(ImVec2(from, aBarPos.y + backgroundTileSize), ImVec2(to, aBarPos.y + backgroundTileSize * 2), darkCol);
			draw_list->AddRectFilled(ImVec2(from, aBarPos.y + backgroundTileSize * 2), ImVec2(to, aBarPos.y + backgroundTileSize * 3), lightCol);
		}
		else
		{
			draw_list->AddRectFilled(ImVec2(from, aBarPos.y), ImVec2(to, aBarPos.y + backgroundTileSize), darkCol);
			draw_list->AddRectFilled(ImVec2(from, aBarPos.y + backgroundTileSize), ImVec2(to, aBarPos.y + backgroundTileSize * 2), lightCol);
			draw_list->AddRectFilled(ImVec2(from, aBarPos.y + backgroundTileSize * 2), ImVec2(to, aBarPos.y + backgroundTileSize * 3), darkCol);
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

		ImGui::SetCursorScreenPos(ImVec2(aBarPos.x, aBarPos.y + aHeight + 10.0f));
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
		
		if (ImGui::BeginPopup(aPopupID, /*ImGuiWindowFlags_NoMove | */ImGuiWindowFlags_NoResize))
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
		
		if (ImGui::BeginPopup(aPopupID, /*ImGuiWindowFlags_NoMove | */ImGuiWindowFlags_NoResize))
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
