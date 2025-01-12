#include "InspectorPanel.h"
#include <CommonUtilities/StringUtils.h>
#include <Epoch/ImGui/ImGui.h>
#include <Epoch/Editor/SelectionManager.h>
#include <Epoch/Editor/PanelIDs.h>
#include <Epoch/Rendering/Material.h>
#include <Epoch/Rendering/Texture.h>

namespace Epoch
{
	InspectorPanel::InspectorPanel()
	{
		myDrawFunctions[AssetType::Material] = [this](UUID aAssetID) { DrawMaterialInspector(aAssetID); };
		myDrawFunctions[AssetType::Model] = [this](UUID aAssetID) { DrawModelInspector(aAssetID); };
		myDrawFunctions[AssetType::Prefab] = [this](UUID aAssetID) { DrawPrefabInspector(aAssetID); };
		myDrawFunctions[AssetType::Texture] = [this](UUID aAssetID) { DrawTextureInspector(aAssetID); };
	}

	void InspectorPanel::OnImGuiRender(bool& aIsOpen)
	{
		EPOCH_PROFILE_FUNC();

		ImGui::Begin(INSPECTOR_PANEL_ID, &aIsOpen);
		
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

	void WriteHeader(UUID aAssetID)
	{
		const auto& metadata = Project::GetEditorAssetManager()->GetMetadata(aAssetID);
		const std::string name = std::format("{} ({})", metadata.filePath.stem().string(), AssetTypeToString(metadata.type));

		ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed
			| ImGuiTreeNodeFlags_SpanAvailWidth
			| ImGuiTreeNodeFlags_AllowItemOverlap
			| ImGuiTreeNodeFlags_FramePadding
			| ImGuiTreeNodeFlags_Leaf;

		const float framePaddingX = 4.0f;
		const float framePaddingY = 4.0f; // affects height of the header

		UI::ScopedStyle headerRounding(ImGuiStyleVar_FrameRounding, 0.0f);
		UI::ScopedStyle headerPaddingAndHeight(ImGuiStyleVar_FramePadding, ImVec2{ framePaddingX, framePaddingY });

		ImColor defaultTreeNodeCol = ImGui::GetStyle().Colors[ImGuiCol_Header];

		UI::ScopedColorStack colors =
		{
			ImGuiCol_Header, defaultTreeNodeCol,
			ImGuiCol_HeaderHovered, defaultTreeNodeCol,
			ImGuiCol_HeaderActive, defaultTreeNodeCol,
		};

		UI::Fonts::PushFont("Bold");
		ImGui::TreeNodeEx(name.c_str(), treeNodeFlags, name.c_str());
		ImGui::TreePop();
		UI::Fonts::PopFont();

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

		CU::Color albedoColor = material->GetAlbedoColor();
		if (UI::Property_ColorEdit3("Albedo Color", albedoColor))
		{
			material->SetAlbedoColor(albedoColor.GetVector3());
			modified = true;
		}

		modified |= UI::Property_DragFloat("Roughness", material->GetRoughness(), 0.02f, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
		modified |= UI::Property_DragFloat("Metalness", material->GetMetalness(), 0.02f, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

		{
			UI::ScopedID id("Albedo");
			modified |= UI::Property_AssetReference<Texture2D>("Albedo Texture", material->GetAlbedoTexture());
		}

		{
			UI::ScopedID id("Normal");
			modified |= UI::Property_AssetReference<Texture2D>("Normal Texture", material->GetNormalTexture());
		}

		{
			UI::ScopedID id("Material");
			modified |= UI::Property_AssetReference<Texture2D>("Material Texture", material->GetMaterialTexture());
		}
		
		modified |= UI::Property_DragFloat2("UV Tiling", material->GetUVTiling(), 0.02f);
		modified |= UI::Property_DragFloat("Normal Strength", material->GetNormalStrength(), 0.02f);

		CU::Color emissionColor = material->GetEmissionColor();
		if (UI::Property_ColorEdit3("Emission Color", emissionColor))
		{
			material->SetEmissionColor(emissionColor.GetVector3());
			modified = true;
		}
		modified |= UI::Property_DragFloat("Emission Strength", material->GetEmissionStrength(), 0.02f, 0.0f, 255.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

		UI::EndPropertyGrid();

		if (modified)
		{
			AssetImporter::Serialize(material);
		}
	}

	void InspectorPanel::DrawModelInspector(UUID aAssetID)
	{
		std::shared_ptr<Model> model = AssetManager::GetAssetAsync<Model>(aAssetID);

		if (!model)
		{
			return;
		}

		WriteHeader(aAssetID);

		//ImGui::Text(("Vertices: " + CU::NumberFormat(mesh->GetVertexCount())).c_str());
		//ImGui::Text(("Indices: " + CU::NumberFormat(mesh->GetIndexCount())).c_str());
		//ImGui::Text(("Triangles: " + CU::NumberFormat(mesh->GetTriangleCount())).c_str());
		//
		//UI::Spacing();
		//
		//ImGui::Text("Sub meshes: %u", mesh->GetSubmeshes().size());
		//
		//UI::Spacing();
		//
		//if (mesh->HasSkeleton())
		//{
		//	ImGui::Text("Bones: %u", mesh->GetSkeleton()->GetNumBones());
		//}
		//
		//if (mesh->GetAnimationCount())
		//{
		//	ImGui::Text("Animation Count: %u", mesh->GetAnimationCount());
		//}
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
