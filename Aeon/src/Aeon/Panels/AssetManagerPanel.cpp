#include "AssetManagerPanel.h"
#include <CommonUtilities/StringUtils.h>
#include <Epoch/ImGui/ImGui.h>
#include <Epoch/Debug/Instrumentor.h>
#include <Epoch/Project/Project.h>
#include <Epoch/Editor/PanelIDs.h>

namespace Epoch
{
	enum class SearchType { Default, Path, Type, Id };

	static std::unordered_map<std::string, SearchType> staticSearchTypeMap =
	{
		{ "path",		SearchType::Path },
		{ "Path",		SearchType::Path },
		{ "type",		SearchType::Type },
		{ "Type",		SearchType::Type },
		{ "id",			SearchType::Id },
		{ "Id",			SearchType::Id }
	};

	void AssetManagerPanel::OnImGuiRender(bool& aIsOpen)
	{
		EPOCH_PROFILE_FUNC();

		if (!aIsOpen) return;

		ImGui::Begin(ASSET_MANAGER_PANEL_ID, &aIsOpen);

		static std::string searchString;
		ImGui::InputTextWithHint("##Search", "Search...", &searchString, ImGuiInputTextFlags_AutoSelectAll);
		ImGui::SameLine();
		UI::HelpMarker([]()
			{
				UI::Fonts::PushFont("Bold");
				ImGui::Text("\"Path:\"");
				UI::Fonts::PopFont();
				ImGui::SameLine(65);
				ImGui::Text("To search for asset with specific path");

				UI::Fonts::PushFont("Bold");
				ImGui::Text("\"Type:\"");
				UI::Fonts::PopFont();
				ImGui::SameLine(65);
				ImGui::Text("To search for asset with specific type");

				UI::Fonts::PushFont("Bold");
				ImGui::Text("\"Id:\"");
				UI::Fonts::PopFont();
				ImGui::SameLine(65);
				ImGui::Text("To search for asset with specific ID");
			});

		static bool showMemoryOnlyAssets = false;
		ImGui::Checkbox("Show Memory Only Assets", &showMemoryOnlyAssets);

		ImGui::Separator();

		ImGui::BeginChild("AssetRegView");

		SearchType searchType = SearchType::Default;

		if (!searchString.empty())
		{
			const std::size_t found = searchString.find_first_of(":");
			if (found <= 5)
			{
				auto it = staticSearchTypeMap.find(searchString.substr(0, found));
				if (it != staticSearchTypeMap.end())
				{
					searchType = it->second;
					searchString = searchString.substr(found + 1);
				}
			}
		}

		UI::BeginPropertyGrid();
		ImGui::SetColumnWidth(0, ImGui::CalcTextSize("File Path").x * 2.0f);

		int id = 0;
		for (const auto& [handle, metadata] : Project::GetEditorAssetManager()->GetAssetRegistry())
		{
			if (!showMemoryOnlyAssets && metadata.isMemoryAsset)
			{
				continue;
			}

			bool matchesSearch = false;

			switch (searchType)
			{
			case Epoch::SearchType::Default:
			{
				const std::string assetPath = metadata.filePath.string().c_str();
				if (UI::IsMatchingSearch(assetPath, searchString))
				{
					matchesSearch = true;
					break;
				}

				const std::string assetType = std::string(AssetTypeToString(metadata.type));
				if (UI::IsMatchingSearch(assetType, searchString))
				{
					matchesSearch = true;
					break;
				}

				const std::string assetID = std::to_string(handle);
				if (UI::IsMatchingSearch(assetID, searchString))
				{
					matchesSearch = true;
					break;
				}

				break;
			}
			case Epoch::SearchType::Path:
			{
				const std::string assetPath = metadata.filePath.string().c_str();
				matchesSearch = UI::IsMatchingSearch(assetPath, searchString);
				break;
			}
			case Epoch::SearchType::Type:
			{
				const std::string assetType = std::string(AssetTypeToString(metadata.type));
				matchesSearch = UI::IsMatchingSearch(assetType, searchString);
				break;
			}
			case Epoch::SearchType::Id:
			{
				const std::string assetID = std::to_string(handle);
				matchesSearch = UI::IsMatchingSearch(assetID, searchString);
				break;
			}
			default: break;
			}

			if (!matchesSearch) continue;

			ImGui::PushID(id++);

			if (UI::Property_ClickableText("Handle", std::to_string(handle)))
			{
				ImGui::SetClipboardText(std::to_string(handle).c_str());
			}

			if (UI::Property_ClickableText("File Path", metadata.filePath.string()))
			{
				ImGui::SetClipboardText(metadata.filePath.string().c_str());
			}

			if (UI::Property_ClickableText("Type", AssetTypeToString(metadata.type)))
			{
				ImGui::SetClipboardText(AssetTypeToString(metadata.type));
			}

			//UI::Property_ClickableText("Loaded", metadata.isDataLoaded ? "True" : "False");

			ImGui::Separator();

			ImGui::PopID();
		}

		UI::EndPropertyGrid();

		ImGui::EndChild();

		ImGui::End();
	}
}
