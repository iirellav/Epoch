#include "epch.h"
#include "UICore.h"
#include "Epoch/ImGui/ImGuiFonts.h"
#include "Epoch/ImGui/ImGuiWidgets.h"
#include "Epoch/Editor/GradientEditor.h"
#include "Epoch/Rendering/Texture.h"

namespace Epoch::UI
{
	static int staticUIContextID = 0;
	static int staticCheckboxCount = 0;

	void PushID()
	{
		ImGui::PushID(staticUIContextID++);
	}

	void PopID()
	{
		ImGui::PopID();
		staticUIContextID--;
	}

	void ShiftCursorX(float aDistance)
	{
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + aDistance);
	}

	void ShiftCursorY(float aDistance)
	{
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + aDistance);
	}

	void ShiftCursor(float aX, float aY)
	{
		const ImVec2 cursor = ImGui::GetCursorPos();
		ImGui::SetCursorPos(ImVec2(cursor.x + aX, cursor.y + aY));
	}

	void BeginPropertyGrid(int aColumns)
	{
		PushID();
		//ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f));
		//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));
		ImGui::Columns(aColumns);
	}

	void EndPropertyGrid()
	{
		ImGui::Columns(1);
		//ImGui::PopStyleVar(2); // ItemSpacing, FramePadding
		ImGui::PopStyleVar(); // ItemSpacing
		//ShiftCursorY(18.0f);
		PopID();
	}

	void BeginCheckboxGroup(const char* label)
	{
		ImGui::PushID(label);
		PushID();
		//ShiftCursor(10.0f, 9.0f);
		ShiftCursor(10.0f, 3.0f);
		ImGui::Text(label);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);
	}

	void EndCheckboxGroup()
	{
		ImGui::PopItemWidth();
		ImGui::NextColumn();
		PopID();
		ImGui::PopID();
		staticCheckboxCount = 0;
	}

	bool ColoredButton(const char* aLabel, const ImColor& aBackgroundColor, ImVec2 aButtonSize)
	{
		ScopedColor buttonColor(ImGuiCol_Button, aBackgroundColor);
		return ImGui::Button(aLabel, aButtonSize);
	}

	bool ColoredButton(const char* aLabel, const ImColor& aBackgroundColor, const ImColor& aForegroundColor, ImVec2 aButtonSize)
	{
		ScopedColor textColor(ImGuiCol_Text, aForegroundColor);
		ScopedColor buttonColor(ImGuiCol_Button, aBackgroundColor);
		return ImGui::Button(aLabel, aButtonSize);
	}

	bool TableRowClickable(const char* aId, float aRowHeight)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		window->DC.CurrLineSize.y = aRowHeight;

		ImGui::TableNextRow(0, aRowHeight);
		ImGui::TableNextColumn();

		window->DC.CurrLineTextBaseOffset = 3.0f;
		const ImVec2 rowAreaMin = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), 0).Min;
		const ImVec2 rowAreaMax = { ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), ImGui::TableGetColumnCount() - 1).Max.x, rowAreaMin.y + aRowHeight };

		ImGui::PushClipRect(rowAreaMin, rowAreaMax, false);

		bool isRowHovered, held;
		bool isRowClicked = ImGui::ButtonBehavior(ImRect(rowAreaMin, rowAreaMax), ImGui::GetID(aId), &isRowHovered, &held, ImGuiButtonFlags_AllowOverlap);

		ImGui::SetItemAllowOverlap();
		ImGui::PopClipRect();

		return isRowClicked;
	}

	bool IsInputEnabled()
	{
		const auto& io = ImGui::GetIO();
		return (io.ConfigFlags & ImGuiConfigFlags_NoMouse) == 0 && (io.ConfigFlags & ImGuiConfigFlags_NavNoCaptureKeyboard) == 0;
	}

	void SetInputEnabled(bool aEnabled)
	{
		auto& io = ImGui::GetIO();

		if (aEnabled)
		{
			io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
			io.ConfigFlags &= ~ImGuiConfigFlags_NavNoCaptureKeyboard;
		}
		else
		{
			io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
			io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;
		}
	}

	bool IsWindowFocused(const char* aWindowName, const bool aCheckRootWindow)
	{
		ImGuiWindow* currentNavWindow = GImGui->NavWindow;

		if (aCheckRootWindow)
		{
			// Get the actual nav window (not e.g a table)
			ImGuiWindow* lastWindow = NULL;
			while (lastWindow != currentNavWindow)
			{
				lastWindow = currentNavWindow;
				currentNavWindow = currentNavWindow->RootWindow;
			}
		}

		return currentNavWindow == ImGui::FindWindowByName(aWindowName);
	}


	void Spacing(unsigned aSpace)
	{
		while (aSpace-- > 0)
		{
			ImGui::Spacing();
		}
	}

	void Separator(ImVec2 aSize, ImVec4 aColor)
	{
		ImGui::PushStyleColor(ImGuiCol_ChildBg, aColor);
		ImGui::BeginChild("sep", aSize);
		ImGui::EndChild();
		ImGui::PopStyleColor();
	}

	void HelpMarker(const char* aDesc)
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::BeginItemTooltip())
		{
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(aDesc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	bool PropertyGridHeader(const std::string& aName, bool aOpenByDefault)
	{
		return PropertyGridHeader(aName, aName, aOpenByDefault);
	}

	bool PropertyGridHeader(const std::string& aId, const std::string& aName, bool aOpenByDefault)
	{
		ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed
			| ImGuiTreeNodeFlags_SpanAvailWidth
			| ImGuiTreeNodeFlags_AllowItemOverlap
			| ImGuiTreeNodeFlags_FramePadding;

		if (aOpenByDefault)
		{
			treeNodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
		}

		bool open = false;
		const float framePaddingX = 4.0f;
		const float framePaddingY = 4.0f; // affects height of the header

		UI::ScopedStyle headerRounding(ImGuiStyleVar_FrameRounding, 0.0f);
		UI::ScopedStyle headerPaddingAndHeight(ImGuiStyleVar_FramePadding, ImVec2{ framePaddingX, framePaddingY });

		Fonts::PushFont("Bold");
		open = ImGui::TreeNodeEx(aId.c_str(), treeNodeFlags, aName.c_str());
		Fonts::PopFont();

		return open;
	}

	bool PropertyGridHeaderWithIcon(const std::string& aLabel, const std::shared_ptr<Texture2D>& aIcon, const ImVec2& aSize, bool aOpenByDefault)
	{
		return PropertyGridHeaderWithIconAndCheckbox(aLabel, aIcon, aSize, nullptr, aOpenByDefault);
	}

	bool PropertyGridHeaderWithIconAndCheckbox(const std::string& aLabel, const std::shared_ptr<Texture2D>& aIcon, const ImVec2& aSize, bool* outCheckboxValue, bool aOpenByDefault)
	{
		ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed
			| ImGuiTreeNodeFlags_SpanAvailWidth
			| ImGuiTreeNodeFlags_AllowItemOverlap
			| ImGuiTreeNodeFlags_FramePadding;

		if (aOpenByDefault)
		{
			treeNodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
		}

		bool open = false;
		const float framePaddingX = 4.0f;
		const float framePaddingY = 4.0f; // affects height of the header

		UI::ScopedStyle headerRounding(ImGuiStyleVar_FrameRounding, 0.0f);
		UI::ScopedStyle headerPaddingAndHeight(ImGuiStyleVar_FramePadding, ImVec2{ framePaddingX, framePaddingY });

		ImGui::PushID(aLabel.c_str());

		open = ImGui::TreeNodeEx("##dummy_id", treeNodeFlags, "");

		const float lineHeight = ImGui::GetItemRectMax().y - ImGui::GetItemRectMin().y;

		if (outCheckboxValue)
		{
			ImGui::SameLine();

			UI::ScopedColorStack checkboxBgColor(
				ImGuiCol_FrameBg, IM_COL32(35, 35, 35, 255),
				ImGuiCol_FrameBgHovered, IM_COL32(35, 35, 35, 255),
				ImGuiCol_FrameBgActive, IM_COL32(35, 35, 35, 255));

			UI::ScopedStyle framePadding(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

			UI::ShiftCursorY(4);
			ImGui::Checkbox("##dummy_checkbox_id", outCheckboxValue);
		}

		ImGui::SameLine();
		UI::ShiftCursorY(lineHeight * 0.5f - aSize.y * 0.5f);
		ImGui::Image((ImTextureID)aIcon->GetView(), aSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0));
		ImGui::SameLine();
		Fonts::PushFont("Bold");
		ImGui::TextUnformatted(aLabel.c_str());
		Fonts::PopFont();

		ImGui::PopID();

		return open;
	}

	bool SubHeader(const std::string& aLabel, bool aOpenByDefault)
	{
		//ImGuiTreeNodeFlags treeNodeFlags = /*ImGuiTreeNodeFlags_SpanAvailWidth |*/ ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		//
		//if (aOpenByDefault)
		//{
		//	treeNodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
		//}
		//
		//UI::Fonts::PushFont("Bold");
		//UI::ShiftCursorX(-17.0f);
		//bool open = ImGui::TreeNodeEx(aLabel.c_str(), treeNodeFlags);
		//UI::Fonts::PopFont();
		//
		//return open;

		return SubHeaderWithCheckbox(aLabel, nullptr, aOpenByDefault);
	}

	bool SubHeaderWithCheckbox(const std::string& aLabel, bool* outCheckboxValue, bool aOpenByDefault)
	{
		ImGuiTreeNodeFlags treeNodeFlags = /*ImGuiTreeNodeFlags_SpanAvailWidth |*/ ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_AllowItemOverlap;

		if (aOpenByDefault)
		{
			treeNodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
		}

		ImGui::PushID(aLabel.c_str());
		
		UI::ShiftCursorX(-17.0f);
		bool open = ImGui::TreeNodeEx("##dummy_id", treeNodeFlags, "");

		const float lineHeight = ImGui::GetItemRectMax().y - ImGui::GetItemRectMin().y;

		if (outCheckboxValue)
		{
			ImGui::SameLine();

			ImVec4 tempCol = ImGui::GetStyle().Colors[ImGuiCol_Header];

			UI::ScopedColorStack checkboxBgColor(
				ImGuiCol_FrameBg, tempCol,
				ImGuiCol_FrameBgHovered, tempCol,
				ImGuiCol_FrameBgActive, tempCol);

			UI::ScopedStyle framePadding(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

			UI::ShiftCursorY(4);
			ImGui::Checkbox("##dummy_checkbox_id", outCheckboxValue);
		}

		ImGui::SameLine();
		Fonts::PushFont("Bold");
		ImGui::TextUnformatted(aLabel.c_str());
		Fonts::PopFont();

		ImGui::PopID();

		return open;
	}


	bool Property_Checkbox(const char* aLabel, bool& outValue, const char* aTooltip)
	{
		bool modified = false;

		//ShiftCursor(10.0f, 9.0f);
		ShiftCursor(10.0f, 3.0f);
		ImGui::Text(aLabel);

		if (std::strlen(aTooltip) != 0)
		{
			ImGui::SameLine();
			HelpMarker(aTooltip);
		}

		ImGui::NextColumn();
		//ShiftCursorY(4.0f);
		ImGui::PushItemWidth(-1);

		modified = ImGui::Checkbox(std::format("##{0}", aLabel).c_str(), &outValue);

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	bool Property_GroupCheckbox(const char* aLabel, bool& outValue)
	{
		if (++staticCheckboxCount > 1)
		{
			ImGui::SameLine();
			ShiftCursor(5.0f, -3.0f);
		}
		else
		{
			//ShiftCursorY(9.0f);
			ShiftCursor(5.0f, 3.0f);
		}

		ImGui::Text(aLabel);
		ImGui::SameLine();
		ShiftCursorY(-4.0f);
		return ImGui::Checkbox(("##" + std::string(aLabel)).c_str(), &outValue);
	}

	void Property_Text(const char* aLabel, const std::string& aValue, const char* aTooltip)
	{
		//ShiftCursor(10.0f, 9.0f);
		ShiftCursor(10.0f, 3.0f);
		ImGui::Text(aLabel);

		if (std::strlen(aTooltip) != 0)
		{
			ImGui::SameLine();
			HelpMarker(aTooltip);
		}

		ImGui::NextColumn();
		//ShiftCursorY(4.0f);
		ImGui::PushItemWidth(-1);
		
		ImGui::BeginDisabled();
		ImGui::InputText(std::format("##{0}", aLabel).c_str(), (char*)aValue.c_str(), aValue.size(), ImGuiInputTextFlags_ReadOnly);
		ImGui::EndDisabled();

		ImGui::PopItemWidth();
		ImGui::NextColumn();
	}

	void Property_ColoredText(const char* aLabel, const std::string& aValue, ImU32 aColor, const char* aTooltip)
	{
		//ShiftCursor(10.0f, 9.0f);
		ShiftCursor(10.0f, 3.0f);
		ImGui::Text(aLabel);

		if (std::strlen(aTooltip) != 0)
		{
			ImGui::SameLine();
			HelpMarker(aTooltip);
		}

		ImGui::NextColumn();
		//ShiftCursorY(4.0f);
		ImGui::PushItemWidth(-1);
		
		ImGui::BeginDisabled();
		ImGui::PushStyleColor(ImGuiCol_Text, aColor);
		ImGui::InputText(std::format("##{0}", aLabel).c_str(), (char*)aValue.c_str(), aValue.size(), ImGuiInputTextFlags_ReadOnly);
		ImGui::PopStyleColor();
		ImGui::EndDisabled();

		ImGui::PopItemWidth();
		ImGui::NextColumn();
	}

	bool Property_ClickableText(const char* aLabel, const std::string& aValue, const char* aTooltip)
	{
		bool clicked = false;

		//ShiftCursor(10.0f, 9.0f);
		ShiftCursor(10.0f, 3.0f);
		ImGui::Text(aLabel);

		if (std::strlen(aTooltip) != 0)
		{
			ImGui::SameLine();
			HelpMarker(aTooltip);
		}

		ImGui::NextColumn();
		//ShiftCursorY(4.0f);
		ImGui::PushItemWidth(-1);
		
		ImVec2 cursorPos = ImGui::GetCursorPos();
		ImGui::BeginDisabled();
		ImGui::InputText(std::format("##{0}", aLabel).c_str(), (char*)aValue.c_str(), aValue.size(), ImGuiInputTextFlags_ReadOnly);
		ImGui::EndDisabled();

		ImRect rect = GetItemRect();
		ImGui::SetCursorPos(cursorPos);
		ImGui::InvisibleButton("ClickableText", rect.GetSize());
		if (ImGui::IsItemClicked())
		{
			clicked = true;
		}

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return clicked;
	}

	bool Property_InputText(const char* aLabel, std::string& outValue, ImGuiInputTextFlags aFlags, const char* aTooltip)
	{
		bool modified = false;

		//ShiftCursor(10.0f, 9.0f);
		ShiftCursor(10.0f, 3.0f);
		ImGui::Text(aLabel);

		if (std::strlen(aTooltip) != 0)
		{
			ImGui::SameLine();
			HelpMarker(aTooltip);
		}

		ImGui::NextColumn();
		//ShiftCursorY(4.0f);
		ImGui::PushItemWidth(-1);

		modified = ImGui::InputText(std::format("##{0}", aLabel).c_str(), &outValue, aFlags);

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	bool Property_InputTextMultiline(const char* aLabel, std::string& outValue, const CU::Vector2f& aSize, ImGuiInputTextFlags aFlags, const char* aTooltip)
	{
		bool modified = false;

		//ShiftCursor(10.0f, 9.0f);
		ShiftCursor(10.0f, 3.0f);
		ImGui::Text(aLabel);

		if (std::strlen(aTooltip) != 0)
		{
			ImGui::SameLine();
			HelpMarker(aTooltip);
		}

		ImGui::NextColumn();
		//ShiftCursorY(4.0f);
		ImGui::PushItemWidth(-1);

		modified = ImGui::InputTextMultiline(std::format("##{0}", aLabel).c_str(), &outValue, { aSize.x, aSize.y }, aFlags);

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	bool Property_FilePath(const char* aLabel, std::filesystem::path& outValue, ImGuiInputTextFlags aFlags, const char* aTooltip)
	{
		bool modified = false;

		//ShiftCursor(10.0f, 9.0f);
		ShiftCursor(10.0f, 3.0f);
		ImGui::Text(aLabel);

		if (std::strlen(aTooltip) != 0)
		{
			ImGui::SameLine();
			HelpMarker(aTooltip);
		}

		ImGui::NextColumn();
		//ShiftCursorY(4.0f);
		ImGui::PushItemWidth(-1);
		
		ImVec2 label_size = ImGui::CalcTextSize("...", NULL, true);
		auto& style = ImGui::GetStyle();
		ImVec2 button_size = ImGui::CalcItemSize(ImVec2(0, 0), label_size.x + style.FramePadding.x + style.ItemInnerSpacing.x, label_size.y + style.FramePadding.y);

		std::string stringValue = outValue.string();

		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - button_size.x - style.FramePadding.x - style.ItemInnerSpacing.x);
		ImGui::BeginDisabled();
		ImGui::InputText(std::format("##{0}", aLabel).c_str(), (char*)stringValue.c_str(), stringValue.size(), ImGuiInputTextFlags_ReadOnly);
		ImGui::EndDisabled();

		ImGui::SameLine();

		if (ImGui::Button("..."))
		{
			std::string result = FileSystem::OpenFileDialog().string();
			if (result != "")
			{
				outValue = result;
				modified = true;
			}
		}

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	bool Property_ColorEdit3(const char* aLabel, CU::Color& outValue, ImGuiColorEditFlags aFlags, const char* aTooltip)
	{
		bool modified = false;

		//ShiftCursor(10.0f, 9.0f);
		ShiftCursor(10.0f, 3.0f);
		ImGui::Text(aLabel);

		if (std::strlen(aTooltip) != 0)
		{
			ImGui::SameLine();
			HelpMarker(aTooltip);
		}

		ImGui::NextColumn();
		//ShiftCursorY(4.0f);
		ImGui::PushItemWidth(-1);

		modified = ImGui::ColorEdit3(std::format("##{0}", aLabel).c_str(), &outValue.r, aFlags);

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	bool Property_ColorEdit4(const char* aLabel, CU::Color& outValue, const char* aTooltip)
	{
		bool modified = false;

		//ShiftCursor(10.0f, 9.0f);
		ShiftCursor(10.0f, 3.0f);
		ImGui::Text(aLabel);

		if (std::strlen(aTooltip) != 0)
		{
			ImGui::SameLine();
			HelpMarker(aTooltip);
		}

		ImGui::NextColumn();
		//ShiftCursorY(4.0f);
		ImGui::PushItemWidth(-1);

		modified = ImGui::ColorEdit4(std::format("##{0}", aLabel).c_str(), &outValue.r);

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	bool Property_InputInt(const char* aLabel, int& outValue, int aStep, int aFastStep, int aMin, int aMax, const char* aTooltip)
	{
		bool modified = false;

		//ShiftCursor(10.0f, 9.0f);
		ShiftCursor(10.0f, 3.0f);
		ImGui::Text(aLabel);

		if (std::strlen(aTooltip) != 0)
		{
			ImGui::SameLine();
			HelpMarker(aTooltip);
		}

		ImGui::NextColumn();
		//ShiftCursorY(4.0f);
		ImGui::PushItemWidth(-1);

		modified = ImGui::InputInt(std::format("##{0}", aLabel).c_str(), &outValue, aStep, aFastStep);

		if (aMin != 0 && aMax != 0)
		{
			outValue = CU::Math::Clamp(outValue, aMin, aMax);
		}

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	bool Property_DragInt(const char* aLabel, int& outValue, int aDelta, int aMin, int aMax, const char* aFormat, ImGuiSliderFlags aFlags, const char* aTooltip)
	{
		bool modified = false;

		//ShiftCursor(10.0f, 9.0f);
		ShiftCursor(10.0f, 3.0f);
		ImGui::Text(aLabel);

		if (std::strlen(aTooltip) != 0)
		{
			ImGui::SameLine();
			HelpMarker(aTooltip);
		}

		ImGui::NextColumn();
		//ShiftCursorY(4.0f);
		ImGui::PushItemWidth(-1);

		modified = ImGui::DragInt(std::format("##{0}", aLabel).c_str(), &outValue, (float)aDelta, aMin, aMax, aFormat, aFlags);

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	bool Property_DragUInt(const char* aLabel, uint32_t& outValue, uint32_t aDelta, uint32_t aMin, uint32_t aMax, const char* aFormat, ImGuiSliderFlags aFlags, const char* aTooltip)
	{
		bool modified = false;

		//ShiftCursor(10.0f, 9.0f);
		ShiftCursor(10.0f, 3.0f);
		ImGui::Text(aLabel);

		if (std::strlen(aTooltip) != 0)
		{
			ImGui::SameLine();
			HelpMarker(aTooltip);
		}

		ImGui::NextColumn();
		//ShiftCursorY(4.0f);
		ImGui::PushItemWidth(-1);

		modified = ImGui::DragScalar(std::format("##{0}", aLabel).c_str(), ImGuiDataType_U32, &outValue, (float)aDelta, &aMin, &aMax, aFormat, aFlags);

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	bool Property_DragInt2(const char* aLabel, CU::Vector2i& outValue, int aDelta, int aMin, int aMax, const char* aFormat, ImGuiSliderFlags aFlags, const char* aTooltip)
	{
		bool modified = false;

		//ShiftCursor(10.0f, 9.0f);
		ShiftCursor(10.0f, 3.0f);
		ImGui::Text(aLabel);

		if (std::strlen(aTooltip) != 0)
		{
			ImGui::SameLine();
			HelpMarker(aTooltip);
		}

		ImGui::NextColumn();
		//ShiftCursorY(4.0f);
		ImGui::PushItemWidth(-1);

		modified = ImGui::DragInt2(std::format("##{0}", aLabel).c_str(), &outValue.x, (float)aDelta, aMin, aMax, aFormat, aFlags);

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	bool Property_SliderFloat(const char* aLabel, float& outValue, float aMin, float aMax, const char* aFormat, ImGuiSliderFlags aFlags, const char* aTooltip)
	{
		bool modified = false;

		//ShiftCursor(10.0f, 9.0f);
		ShiftCursor(10.0f, 3.0f);
		ImGui::Text(aLabel);

		if (std::strlen(aTooltip) != 0)
		{
			ImGui::SameLine();
			HelpMarker(aTooltip);
		}

		ImGui::NextColumn();
		//ShiftCursorY(4.0f);
		ImGui::PushItemWidth(-1);

		modified = ImGui::SliderFloat(std::format("##{0}", aLabel).c_str(), &outValue, aMin, aMax, aFormat, aFlags);

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	bool Property_DragFloat(const char* aLabel, float& outValue, float aDelta, float aMin, float aMax, const char* aFormat, ImGuiSliderFlags aFlags, const char* aTooltip)
	{
		bool modified = false;

		//ShiftCursor(10.0f, 9.0f);
		ShiftCursor(10.0f, 3.0f);
		ImGui::Text(aLabel);

		if (std::strlen(aTooltip) != 0)
		{
			ImGui::SameLine();
			HelpMarker(aTooltip);
		}

		ImGui::NextColumn();
		//ShiftCursorY(4.0f);
		ImGui::PushItemWidth(-1);
		
		ImGui::GetStyle().ButtonTextAlign = { 0.0f, 0.5f };
		modified = UI::DragFloat(std::format("##{0}", aLabel).c_str(), &outValue, aDelta, aMin, aMax, aFormat, aFlags);
		ImGui::GetStyle().ButtonTextAlign = { 0.5f, 0.5f };

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	bool Property_DragFloat2(const char* aLabel, CU::Vector2f& outValue, float aDelta, float aMin, float aMax, const char* aFormat, ImGuiSliderFlags aFlags, const char* aTooltip)
	{
		bool modified = false;

		//ShiftCursor(10.0f, 9.0f);
		ShiftCursor(10.0f, 3.0f);
		ImGui::Text(aLabel);

		if (std::strlen(aTooltip) != 0)
		{
			ImGui::SameLine();
			HelpMarker(aTooltip);
		}

		ImGui::NextColumn();
		//ShiftCursorY(4.0f);
		ImGui::PushItemWidth(-1);

		modified = ImGui::DragFloat2(std::format("##{0}", aLabel).c_str(), &outValue.x, aDelta, aMin, aMax, aFormat, aFlags);

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	bool Property_DragFloat3(const char* aLabel, CU::Vector3f& outValue, float aDelta, float aMin, float aMax, const char* aFormat, ImGuiSliderFlags aFlags, const char* aTooltip)
	{
		bool modified = false;

		//ShiftCursor(10.0f, 9.0f);
		ShiftCursor(10.0f, 3.0f);
		ImGui::Text(aLabel);

		if (std::strlen(aTooltip) != 0)
		{
			ImGui::SameLine();
			HelpMarker(aTooltip);
		}

		ImGui::NextColumn();
		//ShiftCursorY(4.0f);
		ImGui::PushItemWidth(-1);

		modified = ImGui::DragFloat3(std::format("##{0}", aLabel).c_str(), &outValue.x, aDelta, aMin, aMax, aFormat, aFlags);

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	bool Property_DragFloat4(const char* aLabel, CU::Vector4f& outValue, float aDelta, float aMin, float aMax, const char* aFormat, ImGuiSliderFlags aFlags, const char* aTooltip)
	{
		bool modified = false;

		//ShiftCursor(10.0f, 9.0f);
		ShiftCursor(10.0f, 3.0f);
		ImGui::Text(aLabel);

		if (std::strlen(aTooltip) != 0)
		{
			ImGui::SameLine();
			HelpMarker(aTooltip);
		}

		ImGui::NextColumn();
		//ShiftCursorY(4.0f);
		ImGui::PushItemWidth(-1);

		modified = ImGui::DragFloat4(std::format("##{0}", aLabel).c_str(), &outValue.x, aDelta, aMin, aMax, aFormat, aFlags);

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	bool Property_LayerMask(const char* aLabel, uint32_t& outValue, const char* aTooltip)
	{
		bool modified = false;

		//ShiftCursor(10.0f, 9.0f);
		ShiftCursor(10.0f, 3.0f);
		ImGui::Text(aLabel);

		if (std::strlen(aTooltip) != 0)
		{
			ImGui::SameLine();
			HelpMarker(aTooltip);
		}

		ImGui::NextColumn();
		//ShiftCursorY(4.0f);
		ImGui::PushItemWidth(-1);

		if (ImGui::BeginMenu("Layer Mask..."))
		{
			const auto& layers = PhysicsLayerManager::GetLayers();

			ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);

			if (ImGui::MenuItem("All..."))
			{
				outValue = 0;
				for (size_t i = 0; i < layers.size(); i++)
				{
					const auto& layer = layers[i];
					if (layer.name == "")
					{
						continue;
					}

					outValue |= layer.bitValue;
				}
				modified = true;
			}
			if (ImGui::MenuItem("None..."))
			{
				outValue = 0;
				modified = true;
			}

			ImGui::Separator();

			for (size_t i = 0; i < layers.size(); i++)
			{
				const auto& layer = layers[i];
				if (layer.name == "")
				{
					continue;
				}

				bool selected = outValue & layer.bitValue;
				if (ImGui::MenuItem(layer.name.c_str(), nullptr, &selected))
				{
					if (selected)
					{
						outValue |= layer.bitValue;
					}
					else
					{
						outValue &= ~layer.bitValue;
					}

					modified = true;
				}
			}
			ImGui::PopItemFlag();

			ImGui::EndMenu();
		}

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	bool Property_Dropdown(const char* aLabel, const char** aOptions, uint32_t aOptionCount, uint32_t& outSelected, const char* aTooltip)
	{
		bool modified = false;

		//ShiftCursor(10.0f, 9.0f);
		ShiftCursor(10.0f, 3.0f);
		ImGui::Text(aLabel);

		if (std::strlen(aTooltip) != 0)
		{
			ImGui::SameLine();
			HelpMarker(aTooltip);
		}

		ImGui::NextColumn();
		//ShiftCursorY(4.0f);
		ImGui::PushItemWidth(-1);

		const char* current = ((GImGui->CurrentItemFlags & ImGuiItemFlags_MixedValue) != 0) ? "---" : aOptions[outSelected];

		if (ImGui::BeginCombo(std::format("##{0}", aLabel).c_str(), current))
		{
			for (uint32_t i = 0; i < aOptionCount; i++)
			{
				const bool is_selected = (aOptions[outSelected] == aOptions[i]);
				if (ImGui::Selectable(aOptions[i], is_selected))
				{
					current = aOptions[i];
					outSelected = i;
					modified = true;
				}
				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	bool Property_Dropdown(const char* aLabel, const std::vector<std::string>& aOptions, uint32_t aOptionCount, uint32_t& outSelected, const char* aTooltip)
	{
		bool modified = false;

		//ShiftCursor(10.0f, 9.0f);
		ShiftCursor(10.0f, 3.0f);
		ImGui::Text(aLabel);

		if (std::strlen(aTooltip) != 0)
		{
			ImGui::SameLine();
			HelpMarker(aTooltip);
		}

		ImGui::NextColumn();
		//ShiftCursorY(4.0f);
		ImGui::PushItemWidth(-1);

		const char* current = ((GImGui->CurrentItemFlags & ImGuiItemFlags_MixedValue) != 0) ? "---" : aOptions[outSelected].c_str();

		const std::string id = "##" + std::string(aLabel);
		if (ImGui::BeginCombo(std::format("##{0}", aLabel).c_str(), current))
		{
			for (uint32_t i = 0; i < aOptionCount; i++)
			{
				if (aOptions[i] == "")
				{
					continue;
				}

				const bool is_selected = (aOptions[outSelected].c_str() == aOptions[i]);
				if (ImGui::Selectable(aOptions[i].c_str(), is_selected))
				{
					current = aOptions[i].c_str();
					outSelected = i;
					modified = true;
				}
				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	bool Property_Gradient(const char* aLabel, CU::Gradient& outGradient, const char* aTooltip)
	{
		bool modified = false;

		//ShiftCursor(10.0f, 9.0f);
		ShiftCursor(10.0f, 3.0f);
		ImGui::Text(aLabel);

		if (std::strlen(aTooltip) != 0)
		{
			ImGui::SameLine();
			HelpMarker(aTooltip);
		}

		ImGui::NextColumn();
		//ShiftCursorY(4.0f);
		ImGui::PushItemWidth(-1);

		float barHeight = 24.0f;
		float barWidth = ImGui::GetContentRegionAvail().x - 1.0f;
		ImVec2 screenPos = ImGui::GetCursorScreenPos();
		if (ImGui::InvisibleButton("GradientBar", ImVec2(barWidth, barHeight)))
		{
			GradientEditor::Get().SetGradientToEdit(&outGradient);
		}
		UI::Widgets::GradientBar(&outGradient, { screenPos.x, screenPos.y }, barWidth, barHeight);
		modified = GradientEditor::Get().OnImGuiRender();

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	bool Property_CubicBezier(const char* aLabel, const CU::Vector2f& aP1, const CU::Vector2f& aP2, const CU::Vector2f& aP3, const CU::Vector2f& aP4, const char* aTooltip)
	{
		bool modified = false;
		
		//ShiftCursor(10.0f, 9.0f);
		ShiftCursor(10.0f, 3.0f);
		ImGui::Text(aLabel);

		if (std::strlen(aTooltip) != 0)
		{
			ImGui::SameLine();
			HelpMarker(aTooltip);
		}

		ImGui::NextColumn();
		//ShiftCursorY(4.0f);
		ImGui::PushItemWidth(-1);

		float barHeight = 24.0f;
		float barWidth = ImGui::GetContentRegionAvail().x;
		ImVec2 screenPos = ImGui::GetCursorScreenPos();
		if (ImGui::InvisibleButton("CubicBezierBar", ImVec2(barWidth, barHeight)))
		{
			//TODO: Implement
			//CubicBezierEditor::Get().SetCurveToEdit(&outCurve);
		}
		UI::Widgets::CubicBezier({ aP1, aP2, aP3, aP4 }, { screenPos.x, screenPos.y }, barWidth, barHeight);
		//modified = CubicBezierEditor::Get().OnImGuiRender();

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}
}