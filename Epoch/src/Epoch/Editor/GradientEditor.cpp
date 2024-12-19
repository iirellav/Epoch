#include "epch.h"
#include "GradientEditor.h"
#include "Epoch/ImGui/UICore.h"
#include "Epoch/ImGui/ImGuiWidgets.h"
#include "Epoch/Utils/YAMLSerializationHelpers.h"
#include "Epoch/Project/Project.h"

namespace Epoch
{
	constexpr float GradientBarHeight = 40.0f;
	constexpr float GradientPresetBarWidth = 150.0f;
	constexpr float GradientPresetBarHeight = 15.0f;
	constexpr float GradientDeleteDiff = 50.0f;

	bool GradientEditor::OnImGuiRender()
	{
		if (!myGradient)
		{
			return false;
		}

		EPOCH_PROFILE_FUNC();

		ImGui::Begin("Gradient Editor", 0, ImGuiWindowFlags_NoDocking);

		bool modified = false;

		if (mySetFocus)
		{
			ImGui::SetWindowFocus();
			mySetFocus = false;
		}

		if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
		{
			Close();
			ImGui::End();
			return false;
		}

		ImGui::Dummy({ 0.0f, 10.0f }); // Offset Y so that the alpha keys fit on top of the bar.

		const ImVec2 barPos = ImGui::GetCursorScreenPos() + ImVec2(5.0f, 0.0f);
		const float maxWidth = CU::Math::Clamp(ImGui::GetContentRegionAvail().x - 10, 10.0f, 2000.0f);
		const float barBottom = barPos.y + GradientBarHeight;
		const float barTop = barPos.y;

		ImGui::InvisibleButton("GradientEditorBar", ImVec2(maxWidth, GradientBarHeight));

		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
		{
			const float xPos = (ImGui::GetIO().MousePos.x - barPos.x) / maxWidth;
			const float yPos = (ImGui::GetIO().MousePos.y - barPos.y) / GradientBarHeight;

			std::shared_ptr<CU::Gradient::Key> newKey;
			if (yPos > 0.5f)
			{
				newKey = std::make_shared<CU::Gradient::Key>(CU::Gradient::KeyType::Color, xPos, myGradient->GetColorAt(xPos));
				myGradient->AddColorKey(newKey);
			}
			else
			{
				newKey = std::make_shared<CU::Gradient::Key>(CU::Gradient::KeyType::Alpha, xPos, myGradient->GetColorAt(xPos).a);
				myGradient->AddAlphaKey(newKey);
			}

			mySelectedKey = newKey;
			myDraggingKey = true;

			modified = true;
		}

		UI::Widgets::GradientBar(myGradient, CU::Vector2f(barPos.x, barPos.y), maxWidth, GradientBarHeight);
		DrawGradientAlphaKeys(barPos, maxWidth);
		DrawGradientColorKeys(barPos, maxWidth);

		if (!ImGui::IsMouseDown(0) && myDraggingKey)
		{
			myDraggingKey = false;
		}

		if (ImGui::IsMouseDragging(0) && myDraggingKey)
		{
			const float increment = ImGui::GetIO().MouseDelta.x / maxWidth;

			if (increment != 0.0f)
			{
				mySelectedKey->time += increment;
				mySelectedKey->time = CU::Math::Clamp01(mySelectedKey->time);

				myGradient->RefreshCache();

				modified = true;
			}

			if (mySelectedKey->type == CU::Gradient::KeyType::Color)
			{
				const float diffY = ImGui::GetIO().MousePos.y - barBottom;

				if (diffY >= GradientDeleteDiff && myGradient->colorKeys.size() != 1)
				{
					myGradient->RemoveColorKey(mySelectedKey);
					mySelectedKey = nullptr;
					myDraggingKey = false;

					modified = true;
				}
			}
			else
			{
				const float diffY = barTop - ImGui::GetIO().MousePos.y;

				if (diffY >= GradientDeleteDiff && myGradient->alphaKeys.size() != 1)
				{
					myGradient->RemoveAlphaKey(mySelectedKey);
					mySelectedKey = nullptr;
					myDraggingKey = false;

					modified = true;
				}
			}
		}

		if (!mySelectedKey && myGradient->colorKeys.size() > 0)
		{
			mySelectedKey = myGradient->colorKeys.front();
		}

		if (mySelectedKey)
		{
			UI::BeginPropertyGrid();

			if (mySelectedKey->type == CU::Gradient::KeyType::Color)
			{
				modified |= UI::Property_ColorEdit3("Color", mySelectedKey->color);
			}
			else
			{
				modified |= UI::Property_DragFloat("Alpha", mySelectedKey->alpha, 0.01f, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			}

			modified |= UI::Property_DragFloat("Location", mySelectedKey->time, 0.01f, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

			UI::EndPropertyGrid();

			if (modified)
			{
				myGradient->RefreshCache();
			}
		}

		if (UI::PropertyGridHeader("Presets"))
		{
			UI::Spacing();

			float saveButtonWidth = ImGui::GetContentRegionAvail().x * 0.5f;
			ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (saveButtonWidth * 0.5f));
			if (ImGui::Button("Save Preset", ImVec2(saveButtonWidth, 25)))
			{
				myPresets.emplace_back(myGradient->CreateCopy());
				SerializePresets();
			}

			UI::Spacing();
			ImGui::Separator();

			UI::ShiftCursorX(-20);
			UI::ScopedColor childBg(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
			ImGui::BeginChild("PresetsWindow", ImGui::GetContentRegionAvail());

			UI::Spacing();

			const float panelWidth = ImGui::GetContentRegionAvail().x;
			const int columnCount = CU::Math::Max(static_cast<int>(panelWidth / GradientPresetBarWidth), 1);
			ImGui::Columns(columnCount, 0, false);

			const float presetBarWidth = (panelWidth / columnCount) - 15.0f;
			bool openPopup = false;
			for (int i = 0; i < (int)myPresets.size(); ++i)
			{
				const CU::Gradient& gradientPreset = myPresets[i];

				ImGui::PushID(i);
				const ImVec2 presetBarPos = ImGui::GetCursorScreenPos();
				if (ImGui::InvisibleButton("GradientPresetBar", ImVec2(presetBarWidth, GradientPresetBarHeight)))
				{
					gradientPreset.CopyTo(*myGradient);
					mySelectedKey = nullptr;
				}

				if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) && ImGui::IsItemClicked(ImGuiMouseButton_Right))
				{
					openPopup = true;
					mySelectedPresetIndex = i;
				}

				UI::Widgets::GradientBar(&gradientPreset, CU::Vector2f(presetBarPos.x, presetBarPos.y), presetBarWidth, GradientPresetBarHeight);
				ImGui::PopID();
				ImGui::NextColumn();
			}

			if (openPopup)
			{
				ImGui::OpenPopup("Preset Popup");
			}

			if (ImGui::BeginPopup("Preset Popup", 0))
			{
				if (ImGui::MenuItem("Delete"))
				{
					myPresets.erase(myPresets.begin() + mySelectedPresetIndex);
					mySelectedPresetIndex = -1;
					SerializePresets();
				}

				ImGui::EndPopup();
			}

			ImGui::Columns(1);
			ImGui::EndChild();
			ImGui::TreePop();
		}

		ImGui::End();

		return modified;
	}

	void GradientEditor::SetGradientToEdit(CU::Gradient* aGradient)
	{
		myGradient = aGradient;
		aGradient ? myIsOpen = true : myIsOpen = false;
		aGradient ? mySetFocus = true : mySetFocus = false;
		mySelectedKey = nullptr;
		myDraggingKey = false;
	}

	void GradientEditor::Close()
	{
		myGradient = nullptr;
		mySelectedKey = nullptr;
		myDraggingKey = false;
		myIsOpen = false;
	}

	void GradientEditor::DrawGradientColorKeys(ImVec2 aBarPos, float aMaxWidth)
	{
		const float barBottom = aBarPos.y + GradientBarHeight;

		ImU32 keyColor = IM_COL32(255.0f, 255.0f, 255.0f, 255);
		const ImU32 keyOutlineColor = IM_COL32(255.0f, 255.0f, 255.0f, 255);
		const ImU32 selectedKeyOutlineColor = Colors::Theme::blue;

		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		for (const auto& key : myGradient->colorKeys)
		{
			keyColor = IM_COL32(key->color.r * 255.0f, key->color.g * 255.0f, key->color.b * 255.0f, 255);
			const float to = aBarPos.x + key->time * aMaxWidth;

			if (key == mySelectedKey)
			{
				draw_list->AddTriangleFilled(ImVec2(to, barBottom - 4), ImVec2(to - 7, barBottom + 7), ImVec2(to + 7, barBottom + 7), selectedKeyOutlineColor);
				draw_list->AddRectFilled(ImVec2(to - 7, barBottom + 7), ImVec2(to + 7, barBottom + 17), selectedKeyOutlineColor, 0.0f, ImDrawFlags_Closed);
			}
			else
			{
				draw_list->AddTriangleFilled(ImVec2(to, barBottom - 4), ImVec2(to - 7, barBottom + 7), ImVec2(to + 7, barBottom + 7), keyOutlineColor);
				draw_list->AddRectFilled(ImVec2(to - 7, barBottom + 7), ImVec2(to + 7, barBottom + 17), keyOutlineColor, 0.0f, ImDrawFlags_Closed);
			}

			draw_list->AddTriangleFilled(ImVec2(to, barBottom), ImVec2(to - 4, barBottom + 10), ImVec2(to + 4, barBottom + 10), keyColor);
			draw_list->AddRectFilled(ImVec2(to - 4, barBottom + 10), ImVec2(to + 4, barBottom + 14), keyColor, 0.0f, ImDrawFlags_Closed);

			ImGui::SetCursorScreenPos(ImVec2(to - 6, barBottom));
			ImGui::InvisibleButton("Key", ImVec2(12, 18));
			if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
			{
				mySelectedKey = key;
				myDraggingKey = true;
			}
		}
	}

	void GradientEditor::DrawGradientAlphaKeys(ImVec2 aBarPos, float aMaxWidth)
	{
		const float barTop = aBarPos.y;

		const ImU32 keyOutlineColor = IM_COL32(255.0f, 255.0f, 255.0f, 255.0f);
		const ImU32 selectedKeyOutlineColor = Colors::Theme::blue;

		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		for (const auto& key : myGradient->alphaKeys)
		{
			ImU32 keyColor = IM_COL32(255.0f * key->alpha, 255.0f * key->alpha, 255.0f * key->alpha, 255.0f);
			const float to = aBarPos.x + key->time * aMaxWidth;
			
			if (key == mySelectedKey)
			{
				draw_list->AddTriangleFilled(ImVec2(to, barTop + 4), ImVec2(to - 7, barTop - 7), ImVec2(to + 7, barTop - 7), selectedKeyOutlineColor);
				draw_list->AddRectFilled(ImVec2(to - 7, barTop - 7), ImVec2(to + 7, barTop - 17), selectedKeyOutlineColor, 0.0f, ImDrawFlags_Closed);
			}
			else
			{
				draw_list->AddTriangleFilled(ImVec2(to, barTop + 4), ImVec2(to - 7, barTop - 7), ImVec2(to + 7, barTop - 7), keyOutlineColor);
				draw_list->AddRectFilled(ImVec2(to - 7, barTop - 7), ImVec2(to + 7, barTop - 17), keyOutlineColor, 0.0f, ImDrawFlags_Closed);
			}

			draw_list->AddTriangleFilled(ImVec2(to, barTop), ImVec2(to - 4, barTop - 10), ImVec2(to + 4, barTop - 10), keyColor);
			draw_list->AddRectFilled(ImVec2(to - 4, barTop - 10), ImVec2(to + 4, barTop - 14), keyColor, 0.0f, ImDrawFlags_Closed);

			ImGui::SetCursorScreenPos(ImVec2(to - 6, barTop - 18));
			ImGui::InvisibleButton("Key", ImVec2(12, 18));
			if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
			{
				mySelectedKey = key;
				myDraggingKey = true;
			}
		}
	}

	void GradientEditor::SerializePresets()
	{
		EPOCH_PROFILE_FUNC();

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Presets" << YAML::Value << YAML::BeginSeq;

		for (const CU::Gradient& gradient : myPresets)
		{
			out << gradient;
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::filesystem::path path = Project::GetProjectDirectory() / "Regs/GradientPresets.epr";

		if (!std::filesystem::exists(path.parent_path()))
		{
			FileSystem::CreateDirectory(path.parent_path());
		}

		std::ofstream fout(path);
		fout << out.c_str();
		fout.close();
	}

	void GradientEditor::DeserializePresets()
	{
		if (!Project::GetActive())
		{
			return;
		}

		std::filesystem::path path = Project::GetProjectDirectory() / "Regs/GradientPresets.epr";
		if (!std::filesystem::exists(path))
		{
			return;
		}

		EPOCH_PROFILE_FUNC();

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(path.string());
		}
		catch (YAML::ParserException e)
		{
			LOG_ERROR("Failed to load .epoch file '{}'\n     {}", path.string(), e.what());
			return;
		}

		YAML::Node presets = data["Presets"];
		for (auto preset : presets)
		{
			CU::Gradient gradient = preset.as<CU::Gradient>();
			myPresets.push_back(gradient);
		}
	}
}
