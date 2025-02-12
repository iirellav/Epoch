#include "InspectorPanel.h"
#include <CommonUtilities/StringUtils.h>
#include <Epoch/ImGui/ImGui.h>
#include <Epoch/Editor/SelectionManager.h>
#include <Epoch/Rendering/Material.h>
#include <Epoch/Rendering/Texture.h>
#include "Aeon/EditorResources.h"

namespace Epoch
{
	InspectorPanel::InspectorPanel(const std::string& aName) : EditorPanel(aName)
	{
		myDrawFunctions[AssetType::Material] = [this](UUID aAssetID) { DrawMaterialInspector(aAssetID); };
		myDrawFunctions[AssetType::PhysicsMaterial] = [this](UUID aAssetID) { DrawPhysicsMaterialInspector(aAssetID); };
		myDrawFunctions[AssetType::Mesh] = [this](UUID aAssetID) { DrawMeshInspector(aAssetID); };
		myDrawFunctions[AssetType::Prefab] = [this](UUID aAssetID) { DrawPrefabInspector(aAssetID); };
		myDrawFunctions[AssetType::Texture] = [this](UUID aAssetID) { DrawTextureInspector(aAssetID); };
		
		myAssetIconMap[AssetType::Mesh]				= EditorResources::ModelIcon;
		myAssetIconMap[AssetType::Texture]			= EditorResources::TextureIcon;
		myAssetIconMap[AssetType::Scene]			= EditorResources::SceneIcon;
		myAssetIconMap[AssetType::ScriptFile]		= EditorResources::ScriptFileIcon;
		myAssetIconMap[AssetType::Prefab]			= EditorResources::PrefabIcon;
		myAssetIconMap[AssetType::Material]			= EditorResources::MaterialIcon;
		myAssetIconMap[AssetType::PhysicsMaterial]	= EditorResources::MaterialIcon;
		myAssetIconMap[AssetType::Video]			= EditorResources::VideoIcon;
		myAssetIconMap[AssetType::Font]				= EditorResources::FontIcon;
	}

	void InspectorPanel::OnImGuiRender(bool& aIsOpen)
	{
		EPOCH_PROFILE_FUNC();

		bool open = ImGui::Begin(myName.c_str(), &aIsOpen);
		
		if (!open)
		{
			ImGui::End();
			return;
		}

		size_t selectionCount = SelectionManager::GetSelectionCount(SelectionContext::ContentBrowser);
		if (selectionCount > 1)
		{
			ImGui::Text(std::format("Inspecting multiple assets at once isn't supported.").c_str());
		}
		else if (selectionCount == 0)
		{
			ImGui::Text("No asset selected.");
		}
		else if (selectionCount == 1)
		{
			const UUID id = SelectionManager::GetSelections(SelectionContext::ContentBrowser)[0];
			const AssetType type = Project::GetEditorAssetManager()->GetAssetType(id);
			if (myDrawFunctions.find(type) != myDrawFunctions.end())
			{
				myDrawFunctions[type](id);
			}
			else
			{
				ImGui::Text("Inspection for this asset type isn't implemented.");
			}
		}

		ImGui::End();
	}

	void InspectorPanel::WriteHeader(UUID aAssetID)
	{
		const ImVec2 iconSize = { 20.0f, 20.0f };

		const auto& metadata = Project::GetEditorAssetManager()->GetMetadata(aAssetID);
		const std::string name = std::format("{} ({})", metadata.filePath.stem().string(), AssetTypeToString(metadata.type));
		const auto& icon = myAssetIconMap[metadata.type];

		ImGuiTreeNodeFlags treeNodeFlags = 
			ImGuiTreeNodeFlags_Framed
			| ImGuiTreeNodeFlags_FramePadding
			| ImGuiTreeNodeFlags_Leaf
			| ImGuiTreeNodeFlags_NoTreePushOnOpen;

		const float framePaddingX = 0.0f;
		const float framePaddingY = 4.0f; // affects height of the header

		UI::ScopedStyle headerRounding(ImGuiStyleVar_FrameRounding, 0.0f);
		UI::ScopedStyle headerPaddingAndHeight(ImGuiStyleVar_FramePadding, ImVec2{ framePaddingX, framePaddingY });

		ImGui::PushID(name.c_str());

		{
			ImColor defaultTreeNodeCol = ImGui::GetStyle().Colors[ImGuiCol_Header];
			UI::ScopedColorStack colors =
			{
				ImGuiCol_Header, defaultTreeNodeCol,
				ImGuiCol_HeaderHovered, defaultTreeNodeCol,
				ImGuiCol_HeaderActive, defaultTreeNodeCol,
			};

			ImGui::TreeNodeEx("##dummy_id", treeNodeFlags, "");
		}

		const float lineHeight = ImGui::GetItemRectMax().y - ImGui::GetItemRectMin().y;

		ImGui::SameLine();
		UI::ShiftCursor(-20.0f, lineHeight * 0.5f - iconSize.y * 0.5f);
		ImGui::Image((ImTextureID)icon->GetView(), iconSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0));
		ImGui::SameLine();
		UI::Fonts::PushFont("Bold");
		ImGui::TextUnformatted(name.c_str());
		UI::Fonts::PopFont();

		ImGui::PopID();

		UI::Spacing();
	}

	void InspectorPanel::DrawMaterialInspector(UUID aAssetID)
	{
		std::shared_ptr<Material> material = AssetManager::GetAsset<Material>(aAssetID);

		if (!material)
		{
			return;
		}

		WriteHeader(aAssetID);

		bool modified = false;

		UI::BeginPropertyGrid();

		modified |= UI::Property_AssetReference<AssetType::Texture>("Albedo Texture", material->GetAlbedoTexture());

		CU::Color albedoColor = material->GetAlbedoColor();
		if (UI::Property_ColorEdit3("Albedo Color", albedoColor))
		{
			material->SetAlbedoColor(albedoColor.GetVector3());
			modified = true;
		}

		UI::EndPropertyGrid();
		UI::Spacing();
		UI::BeginPropertyGrid();

		modified |= UI::Property_AssetReference<AssetType::Texture>("Material Texture", material->GetMaterialTexture());
		modified |= UI::Property_DragFloat("Roughness", material->GetRoughness(), 0.02f, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
		modified |= UI::Property_DragFloat("Metalness", material->GetMetalness(), 0.02f, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
		
		UI::EndPropertyGrid();
		UI::Spacing();
		UI::BeginPropertyGrid();

		CU::Color emissionColor = material->GetEmissionColor();
		if (UI::Property_ColorEdit3("Emission Color", emissionColor))
		{
			material->SetEmissionColor(emissionColor.GetVector3());
			modified = true;
		}
		modified |= UI::Property_DragFloat("Emission Strength", material->GetEmissionStrength(), 0.02f, 0.0f, 255.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
		
		UI::EndPropertyGrid();
		UI::Spacing();
		UI::BeginPropertyGrid();

		modified |= UI::Property_AssetReference<AssetType::Texture>("Normal Texture", material->GetNormalTexture());
		modified |= UI::Property_DragFloat("Normal Strength", material->GetNormalStrength(), 0.02f);
		
		UI::EndPropertyGrid();
		UI::Spacing();
		UI::BeginPropertyGrid();

		modified |= UI::Property_DragFloat2("UV Tiling", material->GetUVTiling(), 0.02f);
		modified |= UI::Property_DragFloat2("UV Offset", material->GetUVOffset(), 0.01f);

		UI::EndPropertyGrid();

		if (modified)
		{
			AssetImporter::Serialize(material);
		}
	}

	void InspectorPanel::DrawPhysicsMaterialInspector(UUID aAssetID)
	{
		std::shared_ptr<PhysicsMaterial> material = AssetManager::GetAsset<PhysicsMaterial>(aAssetID);

		if (!material)
		{
			return;
		}

		WriteHeader(aAssetID);

		bool modified = false;

		UI::BeginPropertyGrid();
		
		float value = material->StaticFriction();
		if (UI::Property_DragFloat("Static Friction", value, 0.02f, 0.0f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp))
		{
			material->StaticFriction(value);
			modified = true;
		}

		
		value = material->DynamicFriction();
		if (UI::Property_DragFloat("Dynamic Friction", value, 0.02f, 0.0f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp))
		{
			material->DynamicFriction(value);
			modified = true;
		}

		
		value = material->Restitution();
		if (UI::Property_DragFloat("Restitution", value, 0.02f, 0.0f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp))
		{
			material->Restitution(value);
			modified = true;
		}

		UI::EndPropertyGrid();

		if (modified)
		{
			AssetImporter::Serialize(material);
		}
	}

	void InspectorPanel::DrawMeshInspector(UUID aAssetID)
	{
		std::shared_ptr<Mesh> mesh = AssetManager::GetAssetAsync<Mesh>(aAssetID);

		if (!mesh)
		{
			return;
		}

		WriteHeader(aAssetID);

		ImGui::Text(("Vertices: " + CU::NumberFormat(mesh->GetVertexCount())).c_str());
		ImGui::Text(("Indices: " + CU::NumberFormat(mesh->GetIndexCount())).c_str());
		ImGui::Text(("Triangles: " + CU::NumberFormat(mesh->GetTriangleCount())).c_str());

		UI::Spacing();

		ImGui::Text("Sub meshes: %u", mesh->GetSubmeshes().size());
		
		UI::Spacing();

		if (mesh->HasSkeleton())
		{
			ImGui::Text("Bones: %u", mesh->GetSkeleton()->GetNumBones());
		}

		if (mesh->GetAnimationCount())
		{
			ImGui::Text("Animation Count: %u", mesh->GetAnimationCount());
		}
	}
	
	void InspectorPanel::DrawPrefabInspector(UUID aAssetID)
	{
		std::shared_ptr<Prefab> prefab = AssetManager::GetAssetAsync<Prefab>(aAssetID);

		if (!prefab)
		{
			return;
		}

		WriteHeader(aAssetID);

		size_t count = prefab->myScene->GetAllEntitiesWith<IDComponent>().size();
		ImGui::Text("Entity Count: %u", (uint32_t)count);
		
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		
		count = prefab->myScene->GetAllEntitiesWith<MeshRendererComponent>().size();
		ImGui::Text("Mesh Renderers: %u", (uint32_t)count);
		
		count = prefab->myScene->GetAllEntitiesWith<SkinnedMeshRendererComponent>().size();
		ImGui::Text("Skinned Mesh Renderers: %u", (uint32_t)count);
		
		count = prefab->myScene->GetAllEntitiesWith<ParticleSystemComponent>().size();
		ImGui::Text("Particle Systems: %u", (uint32_t)count);
		
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		count = prefab->myScene->GetAllEntitiesWith<PointLightComponent>().size();
		ImGui::Text("Point Lights: %u", (uint32_t)count);

		count = prefab->myScene->GetAllEntitiesWith<SpotlightComponent>().size();
		ImGui::Text("Spotlights: %u", (uint32_t)count);
	}

	void InspectorPanel::DrawTextureInspector(UUID aAssetID)
	{
		std::shared_ptr<Texture2D> texture = AssetManager::GetAsset<Texture2D>(aAssetID);

		if (!texture)
		{
			return;
		}

		WriteHeader(aAssetID);

		ImGui::Text("%u x %u", texture->GetWidth(), texture->GetHeight());
	}
}
