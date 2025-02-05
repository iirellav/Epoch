#include "PagePanel.h"
#include <CommonUtilities/Math/CommonMath.hpp>
#include <Epoch/ImGui/ImGui.h>

namespace Epoch
{
	PagePanel::PagePanel(const std::string& aName) : EditorPanel(aName)
	{
	}

	void PagePanel::OnImGuiRender(bool& aIsOpen)
	{
		bool open = ImGui::Begin(myName.c_str(), &aIsOpen);
		
		if (!open)
		{
			ImGui::End();
			return;
		}

		UI::ScopedStyle spacing(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));
		UI::ScopedStyle padding(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));

		ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerV;

		UI::PushID();
		if (ImGui::BeginTable("", 2, tableFlags, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y)))
		{
			ImGui::TableSetupColumn("Page List", 0, 200.0f);
			ImGui::TableSetupColumn("Page Contents", ImGuiTableColumnFlags_WidthStretch);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			DrawPageList();

			ImGui::TableSetColumnIndex(1);
			if (myCurrentPage < myPages.size())
			{
				ImGui::BeginChild("##page_contents", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y));
				myPages[myCurrentPage].renderFunction();
				ImGui::EndChild();
			}
			else
			{
				LOG_ERROR("Invalid preferences page selected!");
			}

			ImGui::EndTable();
		}

		UI::PopID();

		ImGui::End();
	}

	void PagePanel::DrawPageList()
	{
		UI::ScopedColor childBg(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
		if (!ImGui::BeginChild("##page_list")) return;

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		for (uint32_t i = 0; i < myPages.size(); i++)
		{
			const auto& page = myPages[i];

			const char* label = page.name;
			bool selected = i == myCurrentPage;

			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_FramePadding;
			if (selected) flags |= ImGuiTreeNodeFlags_Selected;
			if (ImGui::TreeNodeEx(label, flags))
			{
				ImGui::TreePop();
			}

			if (ImGui::IsItemClicked())
			{
				myCurrentPage = i;
			}
		}

		ImGui::PopStyleVar();

		ImGui::EndChild();
	}
}
