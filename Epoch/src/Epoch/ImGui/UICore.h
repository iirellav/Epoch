#pragma once
#include <CommonUtilities/Math/Vector/Vector.h>
#include <CommonUtilities/Gradient.h>
#include "Epoch/Debug/Instrumentor.h"
#include "Epoch/Editor/FontAwesome.h"
#include "Epoch/ImGui/ImGuiUtilities.h"
#include "Epoch/ImGui/ImGuiWidgets.h"
#include "Epoch/ImGui/Colors.h"
#include "Epoch/Assets/AssetManager.h"
#include "Epoch/Scene/Scene.h"
#include "Epoch/Script/ScriptCache.h"

namespace Epoch
{
	class Texture2D;
}

namespace Epoch::UI
{
	void PushID();
	void PopID();

	void ShiftCursorX(float aDistance);
	void ShiftCursorY(float aDistance);
	void ShiftCursor(float aX, float aY);

	void BeginPropertyGrid(int aColumns = 2);
	void EndPropertyGrid();

	void BeginCheckboxGroup(const char* label);
	void EndCheckboxGroup();

	bool ColoredButton(const char* aLabel, const ImColor& aBackgroundColor, ImVec2 aButtonSize = { 16.0f, 16.0f });
	bool ColoredButton(const char* aLabel, const ImColor& aBackgroundColor, const ImColor& aForegroundColor, ImVec2 aButtonSize = { 16.0f, 16.0f });

	template<typename T>
	static void Table(const char* aTableName, const char** aColumns, uint32_t aColumnCount, const ImVec2& aSize, T aCallback)
	{
		if (aSize.x <= 0.0f || aSize.y <= 0.0f)
		{
			return;
		}

		float edgeOffset = 4.0f;

		ScopedStyle cellPadding(ImGuiStyleVar_CellPadding, ImVec2(4.0f, 0.0f));
		ImColor backgroundColor = ImColor(0.2f, 0.2f, 0.2f, 0.2f);
		const ImColor colRowAlt = ImColor(0.24f, 0.24f, 0.24f, 0.24f);
		ScopedColor rowColor(ImGuiCol_TableRowBg, backgroundColor);
		ScopedColor rowAltColor(ImGuiCol_TableRowBgAlt, colRowAlt);
		ScopedColor tableColor(ImGuiCol_ChildBg, backgroundColor);

		ImGuiTableFlags flags = ImGuiTableFlags_NoPadInnerX
			| ImGuiTableFlags_Resizable
			| ImGuiTableFlags_Reorderable
			| ImGuiTableFlags_ScrollY
			| ImGuiTableFlags_RowBg;

		if (!ImGui::BeginTable(aTableName, aColumnCount, flags, aSize))
		{
			return;
		}

		const float cursorX = ImGui::GetCursorScreenPos().x;

		for (uint32_t i = 0; i < aColumnCount; i++)
		{
			ImGui::TableSetupColumn(aColumns[i]);
		}

		// Headers
		{
			const ImColor activeColor = ImColor(0.26f, 0.26f, 0.26f, 0.26f);
			ScopedColorStack headerCol(ImGuiCol_HeaderHovered, activeColor, ImGuiCol_HeaderActive, activeColor);

			ImGui::TableSetupScrollFreeze(ImGui::TableGetColumnCount(), 1);
			ImGui::TableNextRow(ImGuiTableRowFlags_Headers, 22.0f);

			for (uint32_t i = 0; i < aColumnCount; i++)
			{
				ImGui::TableSetColumnIndex(i);
				const char* columnName = ImGui::TableGetColumnName(i);
				ImGui::PushID(columnName);
				ShiftCursor(edgeOffset * 2.0f, edgeOffset);
				ImGui::TableHeader(columnName);
				ShiftCursor(-edgeOffset * 2.0f, -edgeOffset);
				ImGui::PopID();
			}
			ImGui::SetCursorScreenPos(ImVec2(cursorX, ImGui::GetCursorScreenPos().y));
		}

		aCallback();
		ImGui::EndTable();
	}

	bool TableRowClickable(const char* aId, float aRowHeight);

	bool IsInputEnabled();
	void SetInputEnabled(bool aEnabled);

	bool IsWindowFocused(const char* aWindowName, const bool aCheckRootWindow = true);

	void Spacing(unsigned aSpace = 1);
	void Separator(ImVec2 aSize, ImVec4 aColor);

	void HelpMarker(const char* aDesc);
	template<typename UIFunction>
	void HelpMarker(UIFunction aFunction)
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::BeginItemTooltip())
		{
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			aFunction();
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	bool PropertyGridHeader(const std::string& aName, bool aOpenByDefault = true);

	bool PropertyGridHeader(const std::string& aId, const std::string& aName, bool aOpenByDefault = true);

	bool PropertyGridHeaderWithIcon(const std::string& aLabel, const std::shared_ptr<Texture2D>& aIcon, const ImVec2& aSize, bool aOpenByDefault = true);

	bool PropertyGridHeaderWithIconAndCheckbox(const std::string& aLabel, const std::shared_ptr<Texture2D>& aIcon, const ImVec2& aSize, bool* outCheckboxValue = nullptr, bool aOpenByDefault = true);

	bool SubHeader(const std::string& aLabel, bool aOpenByDefault = true);

	bool SubHeaderWithCheckbox(const std::string& aLabel, bool* outCheckboxValue = nullptr, bool aOpenByDefault = true);


	bool Property_Checkbox(const char* aLabel, bool& outValue, const char* aTooltip = "");

	bool Property_GroupCheckbox(const char* aLabel, bool& outValue);

	void Property_Text(const char* aLabel, const std::string& aValue, const char* aTooltip = "");

	void Property_ColoredText(const char* aLabel, const std::string& aValue, ImU32 aColor, const char* aTooltip = "");

	bool Property_ClickableText(const char* aLabel, const std::string& aValue, const char* aTooltip = "");

	bool Property_InputText(const char* aLabel, std::string& outValue, const char* aTooltip = "");

	bool Property_InputTextMultiline(const char* aLabel, std::string& outValue, const CU::Vector2f& aSize = CU::Vector2f::Zero, ImGuiInputTextFlags aFlags = 0, const char* aTooltip = "");

	bool Property_ColorEdit3(const char* aLabel, CU::Color& outValue, ImGuiColorEditFlags aFlags = 0, const char* aTooltip = "");

	bool Property_ColorEdit4(const char* aLabel, CU::Color& outValue, const char* aTooltip = "");

	bool Property_InputInt(const char* aLabel, int& outValue, int aStep = 1, int aFastStep = 100, int aMin = 0, int aMax = 0, const char* aTooltip = "");

	bool Property_DragInt(const char* aLabel, int& outValue, int aDelta = 1, int aMin = 0, int aMax = 0, const char* aFormat = "%d", ImGuiSliderFlags aFlags = 0, const char* aTooltip = "");

	bool Property_DragUInt(const char* aLabel, uint32_t& outValue, uint32_t aDelta = 1, uint32_t aMin = 0, uint32_t aMax = 0, const char* aFormat = "%d", ImGuiSliderFlags aFlags = 0, const char* aTooltip = "");

	bool Property_DragInt2(const char* aLabel, CU::Vector2i& outValue, int aDelta = 1, int aMin = 0, int aMax = 0, const char* aFormat = "%d", ImGuiSliderFlags aFlags = 0, const char* aTooltip = "");

	bool Property_SliderFloat(const char* aLabel, float& outValue, float aMin = 0.0f, float aMax = 0.0f, const char* aFormat = "%.3f", ImGuiSliderFlags aFlags = 0, const char* aTooltip = "");

	bool Property_DragFloat(const char* aLabel, float& outValue, float aDelta = 0.1f, float aMin = 0.0f, float aMax = 0.0f, const char* aFormat = "%.3f", ImGuiSliderFlags aFlags = 0, const char* aTooltip = "");

	bool Property_DragFloat2(const char* aLabel, CU::Vector2f& outValue, float aDelta = 0.1f, float aMin = 0.0f, float aMax = 0.0f, const char* aFormat = "%.3f", ImGuiSliderFlags aFlags = 0, const char* aTooltip = "");

	bool Property_DragFloat3(const char* aLabel, CU::Vector3f& outValue, float aDelta = 0.1f, float aMin = 0.0f, float aMax = 0.0f, const char* aFormat = "%.3f", ImGuiSliderFlags aFlags = 0, const char* aTooltip = "");

	bool Property_DragFloat4(const char* aLabel, CU::Vector4f& outValue, float aDelta = 0.1f, float aMin = 0.0f, float aMax = 0.0f, const char* aFormat = "%.3f", ImGuiSliderFlags aFlags = 0, const char* aTooltip = "");

	bool Property_Dropdown(const char* aLabel, const char** aOptions, int aOptionCount, int& outSelected, const char* aTooltip = "");

	bool Property_Dropdown(const char* aLabel, const char** aOptions, int aOptionCount, int& outSelected, const bool* aInconsistent, const char* aTooltip = "");

	bool Property_Gradient(const char* aLabel, CU::Gradient& outGradient, const char* aTooltip = "");

	bool Property_CubicBezier(const char* aLabel, const CU::Vector2f& aP1, const CU::Vector2f& aP2, const CU::Vector2f& aP3, const CU::Vector2f& aP4, const char* aTooltip = "");


	static inline std::function<void(AssetHandle)> OnAssetReferenceDoubleClickedCallback;
	static inline std::function<void(UUID)> OnEntityReferenceDoubleClickedCallback;

	struct PropertyAssetReferenceSettings
	{
		bool emptyIsInvalid = false;
		bool showFullFilePath = false;
	};

	template<typename T>
	static bool Property_AssetReference(const char* aLabel, AssetHandle& outHandle, const char* aTooltip = "", const PropertyAssetReferenceSettings& aSettings = {})
	{
		EPOCH_PROFILE_FUNC();

		bool modified = false;

		//ShiftCursor(10.0f, 9.0f);
		ShiftCursor(10.0f, 5.0f);
		ImGui::Text(aLabel);

		if (std::strlen(aTooltip) != 0)
		{
			ImGui::SameLine();
			HelpMarker(aTooltip);
		}

		ImGui::NextColumn();
		//ShiftCursorY(4.0f);
		ImGui::PushItemWidth(-1);

		std::string buttonText = "None";

		bool valid = false;
		if (outHandle == 0)
		{
			if (!aSettings.emptyIsInvalid)
			{
				valid = true;
			}
		}
		else if (AssetManager::IsAssetHandleValid(outHandle))
		{
			const std::filesystem::path& assetPath = Project::GetEditorAssetManager()->GetMetadata(outHandle).filePath;
			buttonText = aSettings.showFullFilePath ? assetPath.string() : assetPath.stem().string();

			auto object = AssetManager::GetAsset<T>(outHandle);
			if (object)
			{
				valid = true;
			}
		}
		else
		{
			buttonText = "Missing";
		}

		if ((GImGui->CurrentItemFlags & ImGuiItemFlags_MixedValue) != 0)
		{
			buttonText = "---";
		}

		std::string assetSearchPopupID = "AssetSearchPopup";

		if (!valid) ImGui::PushStyleColor(ImGuiCol_Text, Colors::Theme::invalid);
		{
			ImVec4 buttonColor = ImGui::GetStyle().Colors[ImGuiCol_Button];
			ImVec4 newButtonColor = ImVec4(0.125f, 0.125f , 0.125f, 1.0f);
			UI::ScopedColorStack colors =
			{
				ImGuiCol_Button, newButtonColor,
				ImGuiCol_ButtonHovered, newButtonColor,
				ImGuiCol_ButtonActive, newButtonColor
			};
			
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			ImVec2 cursorPos = ImGui::GetCursorScreenPos();
			draw_list->AddRectFilled(cursorPos, cursorPos + ImVec2(ImGui::GetContentRegionAvail().x - 25.0f, 24.0f), ImGui::ColorConvertFloat4ToU32(buttonColor));
			
			ShiftCursor(1.0f, 1.0f);
			ImGui::GetStyle().ButtonTextAlign = { 0.0f, 0.5f };
			ImGui::Button(buttonText.c_str(), { ImGui::GetContentRegionAvail().x - 26.0f, 22.0f });
			ImGui::GetStyle().ButtonTextAlign = { 0.5f, 0.5f };
		}
		if (!valid) ImGui::PopStyleColor();

		if (valid && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			//TODO: Highlight asset in content browser
			if (OnAssetReferenceDoubleClickedCallback)
			{
				OnAssetReferenceDoubleClickedCallback(outHandle);
			}
		}

		if (ImGui::BeginDragDropTarget())
		{
			auto data = ImGui::AcceptDragDropPayload("asset_payload");

			if (data)
			{
				AssetHandle assetHandle = *(AssetHandle*)data->Data;
				std::shared_ptr<Asset> asset = AssetManager::GetAsset<Asset>(assetHandle);
				if (asset && asset->GetAssetType() == T::GetStaticType())
				{
					outHandle = assetHandle;
					modified = true;
				}
			}

			ImGui::EndDragDropTarget();
		}
		
		const float itemSpacingX = ImGui::GetStyle().ItemSpacing.x;
		ImGui::GetStyle().ItemSpacing.x = 0.0f;

		ImGui::SameLine();
		bool assetSearchPopupOpen;
		
		{
			ImVec4 newButtonColor = ImVec4(0.0f, 0.0f , 0.0f, 0.0f);
			UI::ScopedColorStack colors =
			{
				ImGuiCol_Button, newButtonColor,
				ImGuiCol_ButtonHovered, newButtonColor,
				ImGuiCol_ButtonActive, newButtonColor,
				ImGuiCol_Text, ImVec4(1.0f, 1.0f , 1.0f, 0.6f)
			};
			
			ShiftCursor(1.0f, -1.0f);
			assetSearchPopupOpen = ImGui::Button(std::format("{}", EP_ICON_SEARCH).c_str(), { 24.0f, 24.0f });
		}

		ImGui::GetStyle().ItemSpacing.x = itemSpacingX;

		if (assetSearchPopupOpen)
		{
			ImGui::OpenPopup(assetSearchPopupID.c_str());
		}


		bool clear = false;
		if (Widgets::AssetSearchPopup(assetSearchPopupID.c_str(), T::GetStaticType(), outHandle, &clear))
		{
			if (clear)
			{
				outHandle = 0;
			}
			else
			{
				auto object = AssetManager::GetAsset<T>(outHandle);
				if (!object)
				{
					outHandle = 0;
				}
			}

			modified = true;
		}

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	static bool Property_EntityReference(const char* aLabel, UUID& outEntityID, std::shared_ptr<Scene> aCurrentScene, const char* aTooltip = "")
	{
		EPOCH_PROFILE_FUNC();

		bool modified = false;

		//ShiftCursor(10.0f, 9.0f);
		ShiftCursor(10.0f, 5.0f);
		ImGui::Text(aLabel);

		if (std::strlen(aTooltip) != 0)
		{
			ImGui::SameLine();
			HelpMarker(aTooltip);
		}

		ImGui::NextColumn();
		//ShiftCursorY(4.0f);
		ImGui::PushItemWidth(-1);

		bool valid = true;
		std::string buttonText = "None";

		Entity entity = aCurrentScene->TryGetEntityWithUUID(outEntityID);
		if (entity)
		{
			buttonText = entity.GetComponent<NameComponent>().name;
		}
		else if (outEntityID != 0)
		{
			buttonText = "Missing";
			valid = false;
		}

		if ((GImGui->CurrentItemFlags & ImGuiItemFlags_MixedValue) != 0)
		{
			buttonText = "---";
		}

		std::string entitySearchPopupID = "EntitySearchPopup";

		if (!valid) ImGui::PushStyleColor(ImGuiCol_Text, Colors::Theme::invalid);
		{
			ImVec4 buttonColor = ImGui::GetStyle().Colors[ImGuiCol_Button];
			ImVec4 newButtonColor = ImVec4(0.125f, 0.125f , 0.125f, 1.0f);
			UI::ScopedColorStack colors =
			{
				ImGuiCol_Button, newButtonColor,
				ImGuiCol_ButtonHovered, newButtonColor,
				ImGuiCol_ButtonActive, newButtonColor
			};
			
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			ImVec2 cursorPos = ImGui::GetCursorScreenPos();
			draw_list->AddRectFilled(cursorPos, cursorPos + ImVec2(ImGui::GetContentRegionAvail().x - 25.0f, 24.0f), ImGui::ColorConvertFloat4ToU32(buttonColor));

			ShiftCursor(1.0f, 1.0f);
			ImGui::GetStyle().ButtonTextAlign = { 0.0f, 0.5f };
			ImGui::Button(buttonText.c_str(), { ImGui::GetContentRegionAvail().x - 26.0f, 22.0f });
			ImGui::GetStyle().ButtonTextAlign = { 0.5f, 0.5f };
		}
		if (!valid) ImGui::PopStyleColor();

		if (valid && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			//TODO: Highlight asset in content browser
			if (OnEntityReferenceDoubleClickedCallback)
			{
				OnEntityReferenceDoubleClickedCallback(outEntityID);
			}
		}

		if (ImGui::BeginDragDropTarget())
		{
			auto data = ImGui::AcceptDragDropPayload("scene_entity_hierarchy");
			if (data)
			{
				outEntityID = *(UUID*)data->Data;
				modified = true;
			}

			ImGui::EndDragDropTarget();
		}

		const float itemSpacingX = ImGui::GetStyle().ItemSpacing.x;
		ImGui::GetStyle().ItemSpacing.x = 0.0f;

		ImGui::SameLine();
		bool assetSearchPopupOpen;
		
		{
			const ImVec4 newButtonColor = ImVec4(0.0f, 0.0f , 0.0f, 0.0f);
			UI::ScopedColorStack colors =
			{
				ImGuiCol_Button, newButtonColor,
				ImGuiCol_ButtonHovered, newButtonColor,
				ImGuiCol_ButtonActive, newButtonColor,
				ImGuiCol_Text, ImVec4(1.0f, 1.0f , 1.0f, 0.6f)
			};
			
			ShiftCursor(1.0f, -1.0f);
			assetSearchPopupOpen = ImGui::Button(std::format("{}", EP_ICON_SEARCH).c_str(), { 24.0f, 24.0f });
		}

		ImGui::GetStyle().ItemSpacing.x = itemSpacingX;

		if (assetSearchPopupOpen)
		{
			ImGui::OpenPopup(entitySearchPopupID.c_str());
		}

		bool clear = false;
		if (Widgets::EntitySearchPopup(entitySearchPopupID.c_str(), aCurrentScene, outEntityID, &clear))
		{
			if (clear)
			{
				outEntityID = 0;
			}

			modified = true;
		}

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}


	static bool DrawFieldValue(std::shared_ptr<Scene> aSceneContext, const std::string& aFieldName, std::shared_ptr<FieldStorage> aStorage, const char* aTooltip = "")
	{
		if (!aStorage)
		{
			return false;
		}

		const FieldInfo* field = aStorage->GetFieldInfo();

		float min = 0.0f;
		float max = 0.0f;
		float delta = 0.1f;

		std::string id = fmt::format("{}-{}", aFieldName, field->id);
		ImGui::PushID(id.c_str());

		bool result = false;
		if (field->flags & (uint64_t)FieldFlag::ReadOnly)
		{
			ImGui::BeginDisabled();
		}
		switch (field->type)
		{
		case FieldType::Bool:
		{
			bool value = aStorage->GetValue<bool>();
			if (Property_Checkbox(aFieldName.c_str(), value, aTooltip))
			{
				aStorage->SetValue(value);
				result = true;
			}
			break;
		}
		case FieldType::Int32:
		{
			int32_t value = aStorage->GetValue<int32_t>();
			if (Property_DragInt(aFieldName.c_str(), value, 1, 0, 0, "%d", 0, aTooltip))
			{
				aStorage->SetValue(value);
				result = true;
			}
			break;
		}
		case FieldType::Float:
		{
			float value = aStorage->GetValue<float>();
			if (Property_DragFloat(aFieldName.c_str(), value, 0.1f, 0.0f, 0.0f, "%.3f", 0, aTooltip))
			{
				aStorage->SetValue(value);
				result = true;
			}
			break;
		}
		case FieldType::String:
		{
			std::string value = aStorage->GetValue<std::string>();
			if (Property_InputText(aFieldName.c_str(), value, aTooltip))
			{
				aStorage->SetValue<std::string>(value);
				result = true;
			}
			break;
		}
		case FieldType::Vector2:
		{
			CU::Vector2f value = aStorage->GetValue<CU::Vector2f>();
			if (Property_DragFloat2(aFieldName.c_str(), value, 0.1f, 0.0f, 0.0f, "%.3f", 0, aTooltip))
			{
				aStorage->SetValue(value);
				result = true;
			}
			break;
		}
		case FieldType::Vector3:
		{
			CU::Vector3f value = aStorage->GetValue<CU::Vector3f>();
			if (Property_DragFloat3(aFieldName.c_str(), value, 0.1f, 0.0f, 0.0f, "%.3f", 0, aTooltip))
			{
				aStorage->SetValue(value);
				result = true;
			}
			break;
		}
		case FieldType::Color:
		{
			CU::Color value = aStorage->GetValue<CU::Color>();
			if (Property_ColorEdit4(aFieldName.c_str(), value, aTooltip))
			{
				aStorage->SetValue(value);
				result = true;
			}
			break;
		}
		case FieldType::Scene:
		{
			AssetHandle handle = aStorage->GetValue<AssetHandle>();
			if (Property_AssetReference<Scene>(aFieldName.c_str(), handle, aTooltip, { true }))
			{
				aStorage->SetValue(handle);
				result = true;
			}
			break;
		}
		case FieldType::Prefab:
		{
			AssetHandle handle = aStorage->GetValue<AssetHandle>();
			if (Property_AssetReference<Prefab>(aFieldName.c_str(), handle, aTooltip, { true }))
			{
				aStorage->SetValue(handle);
				result = true;
			}
			break;
		}
		case FieldType::Material:
		{
			AssetHandle handle = aStorage->GetValue<AssetHandle>();
			if (Property_AssetReference<Material>(aFieldName.c_str(), handle, aTooltip, { true }))
			{
				aStorage->SetValue(handle);
				result = true;
			}
			break;
		}
		case FieldType::Mesh:
		{
			AssetHandle handle = aStorage->GetValue<AssetHandle>();
			if (Property_AssetReference<Mesh>(aFieldName.c_str(), handle, aTooltip, { true }))
			{
				aStorage->SetValue(handle);
				result = true;
			}
			break;
		}
		case FieldType::Entity:
		{
			UUID uuid = aStorage->GetValue<UUID>();
			if (Property_EntityReference(aFieldName.c_str(), uuid, aSceneContext, aTooltip))
			{
				aStorage->SetValue(uuid);
				result = true;
			}
			break;
		}
		}
		if (field->flags & (uint64_t)FieldFlag::ReadOnly)
		{
			ImGui::EndDisabled();
		}

		ImGui::PopID();

		return result;
	}

	static bool DrawFieldArray(std::shared_ptr<Scene> aSceneContext, const std::string& aFieldName, std::shared_ptr<ArrayFieldStorage> aStorage, const char* aTooltip = "")
	{
		if (!aStorage)
		{
			return false;
		}

		const FieldInfo* field = aStorage->GetFieldInfo();

		bool modified = false;

		ImGui::PushID(aFieldName.c_str());

		uint32_t length = aStorage->GetLength();
		int tempLength = length;

		if (UI::SubHeader(aFieldName.c_str(), false))
		{
			UI::BeginPropertyGrid();

			if (field->flags & (uint64_t)FieldFlag::ReadOnly)
			{
				ImGui::BeginDisabled();
			}

			if (UI::Property_InputInt("Length", tempLength, 1, 1, 0, INT32_MAX))
			{
				aStorage->Resize((uint32_t)tempLength);
				length = tempLength;
				modified = true;
			}

			for (uint32_t i = 0; i < (uint32_t)length; i++)
			{
				std::string idString = fmt::format("{1}-{0}", aFieldName, i);
				std::string indexString = fmt::format("Element {0}", i);

				ImGui::PushID(idString.c_str());

				switch (field->type)
				{
				case FieldType::Bool:
				{
					bool value = aStorage->GetValue<bool>(i);
					if (Property_Checkbox(indexString.c_str(), value, aTooltip))
					{
						aStorage->SetValue(i, value);
						modified = true;
					}
					break;
				}
				case FieldType::Int32:
				{
					int32_t value = aStorage->GetValue<int32_t>(i);
					if (Property_DragInt(indexString.c_str(), value, 1, 0, 0, "%d", 0, aTooltip))
					{
						aStorage->SetValue(i, value);
						modified = true;
					}
					break;
				}
				case FieldType::Float:
				{
					float value = aStorage->GetValue<float>(i);
					if (Property_DragFloat(indexString.c_str(), value, 0.1f, 0.0f, 0.0f, "%.3d", 0, aTooltip))
					{
						aStorage->SetValue(i, value);
						modified = true;
					}
					break;
				}
				case FieldType::String:
				{
					std::string value = aStorage->GetValue<std::string>(i);
					if (Property_InputText(indexString.c_str(), value, aTooltip))
					{
						aStorage->SetValue<std::string>(i, value);
						modified = true;
					}
					break;
				}
				case FieldType::Vector2:
				{
					CU::Vector2f value = aStorage->GetValue<CU::Vector2f>(i);
					if (Property_DragFloat2(indexString.c_str(), value, 0.1f, 0.0f, 0.0f, "%.3f", 0, aTooltip))
					{
						aStorage->SetValue(i, value);
						modified = true;
					}
					break;
				}
				case FieldType::Vector3:
				{
					CU::Vector3f value = aStorage->GetValue<CU::Vector3f>(i);
					if (Property_DragFloat3(indexString.c_str(), value, 0.1f, 0.0f, 0.0f, "%.3f", 0, aTooltip))
					{
						aStorage->SetValue(i, value);
						modified = true;
					}
					break;
				}
				case FieldType::Color:
				{
					CU::Color value = aStorage->GetValue<CU::Color>(i);
					if (Property_ColorEdit4(indexString.c_str(), value, aTooltip))
					{
						aStorage->SetValue(i, value);
						modified = true;
					}
					break;
				}
				case FieldType::Scene:
				{
					AssetHandle handle = aStorage->GetValue<AssetHandle>(i);
					if (Property_AssetReference<Scene>(indexString.c_str(), handle, aTooltip, { true }))
					{
						aStorage->SetValue(i, handle);
						modified = true;
					}
					break;
				}
				case FieldType::Prefab:
				{
					AssetHandle handle = aStorage->GetValue<AssetHandle>(i);
					if (Property_AssetReference<Prefab>(indexString.c_str(), handle, aTooltip, { true }))
					{
						aStorage->SetValue(i, handle);
						modified = true;
					}
					break;
				}
				case FieldType::Material:
				{
					AssetHandle handle = aStorage->GetValue<AssetHandle>(i);
					if (Property_AssetReference<Material>(indexString.c_str(), handle, aTooltip, { true }))
					{
						aStorage->SetValue(i, handle);
						modified = true;
					}
					break;
				}
				case FieldType::Mesh:
				{
					AssetHandle handle = aStorage->GetValue<AssetHandle>(i);
					if (Property_AssetReference<Mesh>(indexString.c_str(), handle, aTooltip, { true }))
					{
						aStorage->SetValue(i, handle);
						modified = true;
					}
					break;
				}
				case FieldType::Entity:
				{
					UUID uuid = aStorage->GetValue<UUID>(i);
					if (Property_EntityReference(indexString.c_str(), uuid, aSceneContext, aTooltip))
					{
						aStorage->SetValue(i, uuid);
						modified = true;
					}
					break;
				}
				}

				ImGui::PopID();
			}

			UI::EndPropertyGrid();

			if (field->flags & (uint64_t)FieldFlag::ReadOnly)
			{
				ImGui::EndDisabled();
			}

			//ImGui::TreePop();
		}

		ImGui::PopID();

		return modified;
	}
}
