#pragma once
#include <memory>
#include <string>
#include <functional>
#include <Epoch/Scene/Scene.h>
#include <Epoch/Scene/Entity.h>
#include <Epoch/ImGui/ImGui.h>
#include <Epoch/Editor/SelectionManager.h>
#include <Epoch/Editor/EditorPanel.h>
#include "Aeon/EditorResources.h"

namespace Epoch
{
	enum class VectorAxis
	{
		X = 1u << 0,
		Y = 1u << 1,
		Z = 1u << 2,
		W = 1u << 3
	};

	class SceneHierarchyPanel : public EditorPanel
	{
	public:
		struct EntityPopupPlugin
		{
			std::function<void(Entity)> function;
			const std::string name;

			EntityPopupPlugin(const std::string& aName, const std::function<void(Entity)>& aFunc) : function(aFunc), name(aName) {}
		};

	public:
		SceneHierarchyPanel();
		~SceneHierarchyPanel() override = default;

		void OnImGuiRender(bool& aIsOpen) override;
		void OnSceneChanged(const std::shared_ptr<Scene>& aScene);

		void SetEntityCreationCallback(const std::function<void(Entity)>& aFunc) { myEntityCreationCallback = aFunc; }
		void AddEntityPopupPlugin(const std::string& aName, const std::function<void(Entity)>& aFunc) { myEntityPopupPlugins.emplace_back(aName, aFunc); }

	private:
		bool ChildMatchesSearchRecursively(Entity aEntity, const std::string& aSearchFilter);
		bool IsAnyDescendantSelected(Entity aEntity, bool aFirst = false);

		void DrawEntityNode(Entity aEntity, const std::string& aSearchFilter);
		void DrawComponents(const std::vector<UUID>& aEntityIDs);

		template<typename TVectorType, typename TComponent, typename GetOtherFunc>
		uint32_t GetInconsistentVectorAxis(GetOtherFunc aFunc)
		{
			uint32_t axes = 0;

			const auto& entities = SelectionManager::GetSelections(SelectionContext::Entity);

			if (entities.size() < 2) return 0;

			Entity firstEntity = myContext->GetEntityWithUUID(entities[0]);
			const TVectorType& first = aFunc(firstEntity.GetComponent<TComponent>());

			for (size_t i = 1; i < entities.size(); i++)
			{
				Entity entity = myContext->GetEntityWithUUID(entities[i]);
				const TVectorType& otherVector = aFunc(entity.GetComponent<TComponent>());

				if (CU::Math::Abs(otherVector.x - first.x) > CU::Math::Epsilon)
				{
					axes |= (uint32_t)VectorAxis::X;
				}

				if (CU::Math::Abs(otherVector.y - first.y) > CU::Math::Epsilon)
				{
					axes |= (uint32_t)VectorAxis::Y;
				}

				if constexpr (std::is_same_v<TVectorType, CU::Vector3f> || std::is_same_v<TVectorType, CU::Vector4f>)
				{
					if (CU::Math::Abs(otherVector.z - first.z) > CU::Math::Epsilon)
					{
						axes |= (uint32_t)VectorAxis::Z;
					}
				}

				if constexpr (std::is_same_v<TVectorType, CU::Vector4f>)
				{
					if (CU::Math::Abs(otherVector.w - first.w) > CU::Math::Epsilon)
					{
						axes |= (uint32_t)VectorAxis::W;
					}
				}
			}

			return axes;
		}

		template<typename TVectorType>
		uint32_t GetInconsistentVectorAxis(const TVectorType& aFirst, const TVectorType& aOther)
		{
			uint32_t axes = 0;

			if (CU::Math::Abs(aOther.x - aFirst.x) > CU::Math::Epsilon)
			{
				axes |= (uint32_t)VectorAxis::X;
			}

			if (CU::Math::Abs(aOther.y - aFirst.y) > CU::Math::Epsilon)
			{
				axes |= (uint32_t)VectorAxis::Y;
			}

			if constexpr (std::is_same_v<TVectorType, CU::Vector3f> || std::is_same_v<TVectorType, CU::Vector4f>)
			{
				if (CU::Math::Abs(aOther.z - aFirst.z) > CU::Math::Epsilon)
				{
					axes |= (uint32_t)VectorAxis::Z;
				}
			}

			if constexpr (std::is_same_v<TVectorType, CU::Vector4f>)
			{
				if (CU::Math::Abs(aOther.w - aFirst.w) > CU::Math::Epsilon)
				{
					axes |= (uint32_t)VectorAxis::W;
				}
			}

			return axes;
		}

		template<typename TPrimitive, typename TComponent, typename GetOtherFunc>
		bool IsInconsistentPrimitive(GetOtherFunc aFunc)
		{
			const auto& entities = SelectionManager::GetSelections(SelectionContext::Entity);

			if (entities.size() < 2) return false;

			Entity firstEntity = myContext->GetEntityWithUUID(entities[0]);
			const TPrimitive& first = aFunc(firstEntity.GetComponent<TComponent>());

			for (size_t i = 1; i < entities.size(); i++)
			{
				Entity entity = myContext->GetEntityWithUUID(entities[i]);

				if (!entity.HasComponent<TComponent>()) continue;

				const auto& otherValue = aFunc(entity.GetComponent<TComponent>());
				if (otherValue != first) return true;
			}

			return false;
		}

		template<typename TComponent, typename UIFunction>
		void DrawComponent(const std::string& aName, UIFunction aUIFunction, std::shared_ptr<Texture2D> aIcon = nullptr)
		{
			std::string name = aName;

			bool shouldDraw = true;
			auto& entities = SelectionManager::GetSelections(SelectionContext::Entity);
			for (const auto& entityID : entities)
			{
				Entity entity = myContext->GetEntityWithUUID(entityID);
				if (!entity.HasComponent<TComponent>())
				{
					shouldDraw = false;
					break;
				}
			}

			if constexpr (std::is_same_v<TComponent, ScriptComponent>)
			{
				if (shouldDraw && IsInconsistentPrimitive<AssetHandle, ScriptComponent>([](const ScriptComponent& aOther) { return aOther.scriptClassHandle; }))
				{
					shouldDraw = false;
				}
			}

			if (!shouldDraw || entities.size() == 0) return;

			Entity firstEntity = myContext->GetEntityWithUUID(entities[0]);
			auto& component = firstEntity.GetComponent<TComponent>();

			if constexpr (std::is_same_v<TComponent, ScriptComponent>)
			{
				if (Project::GetEditorAssetManager()->IsAssetHandleValid(component.scriptClassHandle))
				{
					name = Project::GetEditorAssetManager()->GetMetadata(component.scriptClassHandle).filePath.string() + " (Script)";
				}
			}

			if (aIcon == nullptr) aIcon = EditorResources::QuestionMarkIcon;

			UI::ScopedID compID((void*)typeid(TComponent).hash_code());

			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::Separator();
			bool open = false;

			if constexpr (
				std::is_same_v<TComponent, CameraComponent> ||
				std::is_same_v<TComponent, ParticleSystemComponent> ||
				std::is_same_v<TComponent, MeshRendererComponent> ||
				std::is_same_v<TComponent, SkinnedMeshRendererComponent> ||
				std::is_same_v<TComponent, SpriteRendererComponent> ||
				std::is_same_v<TComponent, VideoPlayerComponent> ||
				std::is_same_v<TComponent, TextRendererComponent> ||
				std::is_same_v<TComponent, SkyLightComponent> ||
				std::is_same_v<TComponent, DirectionalLightComponent> ||
				std::is_same_v<TComponent, SpotlightComponent> ||
				std::is_same_v<TComponent, PointLightComponent> ||
				std::is_same_v<TComponent, ScriptComponent> ||
				std::is_same_v<TComponent, VolumeComponent>
				)
			{
				open = UI::PropertyGridHeaderWithIconAndCheckbox(name, aIcon, { 18.0f, 18.0f }, &component.isActive);
			}
			else
			{
				open = UI::PropertyGridHeaderWithIcon(name, aIcon, { 18.0f, 18.0f });
			}

			bool rightClicked = ImGui::IsItemClicked(ImGuiMouseButton_Right);
			float lineHeight = ImGui::GetItemRectMax().y - ImGui::GetItemRectMin().y;

			ImGui::SameLine(contentRegionAvailable.x - lineHeight);
			UI::ShiftCursorY(lineHeight * 0.25f - 4);
			if (ImGui::InvisibleButton("##ComponentSettings", ImVec2(lineHeight + 8, lineHeight + 8)) || rightClicked)
			{
				ImGui::OpenPopup("ComponentSettings");
			}
			UI::Draw::DrawImage(EditorResources::VerticalEllipsisIcon,
				IM_COL32(255, 255, 255, 200), IM_COL32(255, 255, 255, 255), IM_COL32(255, 255, 255, 150),
				UI::RectExpanded(UI::GetItemRect(), -5.0f, -5.0f));

			bool resetComponent = false;
			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Copy"))
				{
					Scene::CopyComponentFromScene<TComponent>(myContext, firstEntity, myComponentCopyScene, myComponentCopyEntity);
				}

				if (ImGui::MenuItem("Paste"))
				{
					for (auto entityID : SelectionManager::GetSelections(SelectionContext::Entity))
					{
						Entity selectedEntity = myContext->GetEntityWithUUID(entityID);
						Scene::CopyComponentFromScene<TComponent>(myComponentCopyScene, myComponentCopyEntity, myContext, selectedEntity);
					}
				}

				if (ImGui::MenuItem("Reset component"))
				{
					resetComponent = true;
				}

				if (ImGui::MenuItem("Remove component", 0, false, !std::is_same<TComponent, TransformComponent>::value))
				{
					removeComponent = true;
				}

				ImGui::EndPopup();
			}

			if (open)
			{
				Entity entity = myContext->GetEntityWithUUID(entities[0]);
				TComponent& firstComponent = entity.GetComponent<TComponent>();
				const bool isMultiEdit = entities.size() > 1;
				aUIFunction(firstComponent, entities, isMultiEdit);
				ImGui::TreePop();
			}

			if (resetComponent)
			{
				for (auto entityID : SelectionManager::GetSelections(SelectionContext::Entity))
				{
					Entity entity = myContext->GetEntityWithUUID(entityID);
					if (entity.HasComponent<TComponent>())
					{
						entity.RemoveComponent<TComponent>();
						entity.AddComponent<TComponent>();
					}
				}
			}

			if (removeComponent)
			{
				for (auto entityID : SelectionManager::GetSelections(SelectionContext::Entity))
				{
					Entity entity = myContext->GetEntityWithUUID(entityID);
					if (entity.HasComponent<TComponent>())
					{
						entity.RemoveComponent<TComponent>();
					}
				}
			}
		}

		template<typename T>
		void DisplayAddComponentEntry(const std::string& aEntryName, const std::string& aSearchFilter = "");
		
		void DisplayAddScriptComponentEntry(const std::string& aSearchFilter = "");

		template<typename TComponent>
		void DrawMaterialTable(const std::vector<UUID>& aEntityIDs, std::shared_ptr<MaterialTable> aMaterialTable)
		{
			if (UI::SubHeader("Materials", true))
			{
				UI::BeginPropertyGrid();

				const bool isMultiEdit = aEntityIDs.size() > 1;

				int32_t length = 0;

				{
					Entity entity = myContext->GetEntityWithUUID(aEntityIDs[0]);
					auto& mrc = entity.GetComponent<MeshRendererComponent>();
					length = (int32_t)mrc.materialTable->GetMaterialCount();
				}

				int32_t tempLength = length;

				if (UI::Property_InputInt("Length", tempLength, 1, 1, 0, INT32_MAX))
				{
					if (tempLength > length)
					{
						for (auto& entityID : aEntityIDs)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& mrc = entity.GetComponent<MeshRendererComponent>();
							std::shared_ptr<MaterialTable> materialTable = mrc.materialTable;
							materialTable->AddDefaultMaterial();
						}
					}
					else
					{
						for (auto& entityID : aEntityIDs)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& mrc = entity.GetComponent<MeshRendererComponent>();
							std::shared_ptr<MaterialTable> materialTable = mrc.materialTable;
							materialTable->RemoveMaterial();
						}
					}

					length = tempLength;
				}

				for (uint32_t i = 0; i < aMaterialTable->GetMaterialCount(); i++)
				{
					UI::ScopedID matID(i);

					std::string materialName = "Material " + std::to_string(i);
					UUID materialID = aMaterialTable->GetMaterial(i);

					bool inconsistent = IsInconsistentPrimitive<AssetHandle, MeshRendererComponent>([i](const MeshRendererComponent& aOther)
						{
							std::shared_ptr<MaterialTable> materialTable = aOther.materialTable;

							if (i >= materialTable->GetMaterialCount())
							{
								return (AssetHandle)0;
							}

							return materialTable->GetMaterial(i);
						});

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && inconsistent);
					if (UI::Property_AssetReference<Material>(materialName.c_str(), materialID))
					{
						for (auto& entityID : aEntityIDs)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& mrc = entity.GetComponent<MeshRendererComponent>();
							std::shared_ptr<MaterialTable> materialTable = mrc.materialTable;
							if (i < materialTable->GetMaterialCount())
							{
								mrc.materialTable->SetMaterial(i, materialID);
							}
						}
					}
					ImGui::PopItemFlag();
				}

				UI::EndPropertyGrid();

				//ImGui::Spacing();
				//UI::ShiftCursorX(/*ImGui::GetContentRegionAvail().x * 0.5f + */4.0f);
				//if (ImGui::Button("+", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f - 8.0f, 0.0f)))
				//{
				//	for (auto& entityID : aEntityIDs)
				//	{
				//		Entity entity = myContext->GetEntityWithUUID(entityID);
				//		auto& mrc = entity.GetComponent<MeshRendererComponent>();
				//		std::shared_ptr<MaterialTable> materialTable = mrc.materialTable;
				//		materialTable->AddDefaultMaterial();
				//	}
				//}
				//ImGui::SameLine();
				//if (ImGui::Button("-", ImVec2(ImGui::GetContentRegionAvail().x - 4.0f, 0.0f)))
				//{
				//	for (auto& entityID : aEntityIDs)
				//	{
				//		Entity entity = myContext->GetEntityWithUUID(entityID);
				//		auto& mrc = entity.GetComponent<MeshRendererComponent>();
				//		std::shared_ptr<MaterialTable> materialTable = mrc.materialTable;
				//		materialTable->RemoveMaterial();
				//	}
				//}

				//ImGui::TreePop();
			}
		}

		void HierarchyPopup();
		void HandleAssetDrop(Entity aParent);

	private:
		std::shared_ptr<Scene> myContext;

		std::shared_ptr<Scene> myComponentCopyScene;
		Entity myComponentCopyEntity;

		bool myShiftSelectionRunning = false;
		int myFirstSelectedRow = -1;
		int myLastSelectedRow = -1;

		std::function<void(Entity)> myEntityCreationCallback;
		std::vector<EntityPopupPlugin> myEntityPopupPlugins;
	};
}
