#include "EditorLayer.h"
#include <CommonUtilities/Timer.h>
#include <CommonUtilities/Input/InputHandler.h>
#include <CommonUtilities/StringUtils.h>
#include <CommonUtilities/Math/Random.h>
#include <Epoch/Debug/Log.h>
#include <Epoch/Debug/Instrumentor.h>
#include <Epoch/Utils/FileSystem.h>
#include <Epoch/ImGui/ImGui.h>
#include <Epoch/ImGui/UICore.h>
#include <Epoch/ImGui/ImGuizmo.h>
#include <Epoch/Core/Application.h>
#include <Epoch/Core/Window.h>
#include <Epoch/Core/Input.h>
#include <Epoch/Rendering/Renderer.h>
#include <Epoch/Project/ProjectSerializer.h>
#include <Epoch/Project/RuntimeBuilder.h>
#include <Epoch/Scene/SceneSerializer.h>
#include <Epoch/Scene/Prefab.h>
#include <Epoch/Editor/SelectionManager.h>
#include <Epoch/Script/ScriptEngine.h>
#include <Epoch/Script/ScriptBuilder.h>
#include <Epoch/Editor/FontAwesome.h>
#include <Epoch/Editor/GradientEditor.h>
#include <Epoch/Editor/EditorSettings.h>
#include <Epoch/Editor/Console/EditorConsolePanel.h>
#include "EditorResources.h"

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/InspectorPanel.h"
#include "Panels/AssetManagerPanel.h"
#include "Panels/PreferencesPanel.h"
#include "Panels/ProjectSettingsPanel.h"
#include "Panels/ShaderLibraryPanel.h"
#include "Panels/StatisticsPanel.h"
#include "Panels/ScriptEngineDebugPanel.h"

namespace Epoch
{
	static bool staticCreateNewProj = false;
	static bool staticFocusOnViewport = false;

	EditorLayer::EditorLayer() : Layer("Editor Layer")
	{
	}

	void EditorLayer::OnAttach()
	{
		EPOCH_PROFILE_FUNC();

		Log::InitAppConsole();

		EditorSettingsSerializer::Init();
		EditorResources::Init();

		//Set double clicked highlight callbacks
		//UI::OnAssetReferenceDoubleClickedCallback = [](AssetHandle aAssetHandle)
		//	{
		//		SelectionManager::DeselectAll(SelectionContext::Asset);
		//		SelectionManager::Select(SelectionContext::Asset, aAssetHandle);
		//	};
		//
		//UI::OnEntityReferenceDoubleClickedCallback = [](UUID aEntity)
		//	{
		//		SelectionManager::DeselectAll(SelectionContext::Entity);
		//		SelectionManager::Select(SelectionContext::Entity, aEntity);
		//	};

		myPanelManager = std::make_unique<PanelManager>();
		myPanelManager->SetEntityDestroyedCallback([this](Entity aEntity) { OnEntityDeleted(aEntity); });

		std::shared_ptr<SceneHierarchyPanel> sceneHierarchyPanel = myPanelManager->AddPanel<SceneHierarchyPanel>(PanelCategory::View, "Scene Hierarchy", true);
		sceneHierarchyPanel->SetEntityCreationCallback([this](Entity aEntity) { return OnEntityCreated(aEntity); });
		sceneHierarchyPanel->AddEntityPopupPlugin("Set Transform to Editor Camera Transform", [this](Entity aEntity) { OnSetToEditorCameraTransform(aEntity); });
		sceneHierarchyPanel->AddEntityPopupPlugin("Reset Bone Transforms", [this](Entity aEntity) { OnResetBoneTransforms(aEntity); });

		std::shared_ptr<ContentBrowserPanel> contentBrowserPanel = myPanelManager->AddPanel<ContentBrowserPanel>(PanelCategory::View, "Content Browser", true);

		contentBrowserPanel->RegisterItemActivateCallbackForType(AssetType::Scene, [this](const AssetMetadata& aMetadata)
			{
				OpenScene(Project::GetEditorAssetManager()->GetFileSystemPath(aMetadata));
			});

		contentBrowserPanel->RegisterItemActivateCallbackForType(AssetType::ScriptFile, [this](const AssetMetadata& aMetadata)
			{
				FileSystem::OpenExternally(Project::GetEditorAssetManager()->GetFileSystemPath(aMetadata));
			});

		contentBrowserPanel->RegisterNewAssetCreatedCallbackForType(AssetType::ScriptFile, [this](const AssetMetadata& aMetadata)
			{
				ScriptBuilder::RegenerateScriptSolution(Project::GetProjectDirectory());
			});

		contentBrowserPanel->RegisterAssetDeletedCallbackForType(AssetType::ScriptFile, [this](const AssetMetadata& aMetadata)
			{
				ScriptBuilder::RegenerateScriptSolution(Project::GetProjectDirectory());
			});

		contentBrowserPanel->RegistryCurrentSceneRenamedCallback([this](const AssetMetadata& aMetadata)
			{
				OnCurrentSceneRenamed(aMetadata);
			});

		myPanelManager->AddPanel<ProjectSettingsPanel>(PanelCategory::Edit, "Project Settings", false);
		myPanelManager->AddPanel<PreferencesPanel>(PanelCategory::Edit, "Preferences", false);

		myPanelManager->AddPanel<InspectorPanel>(PanelCategory::View, "Inspector", true);
		myPanelManager->AddPanel<EditorConsolePanel>(PanelCategory::View, "Console", true);
		myPanelManager->AddPanel<AssetManagerPanel>(PanelCategory::View, "Asset Manager", false);
		myPanelManager->AddPanel<ShaderLibraryPanel>(PanelCategory::View, "Shader Library", false);
		myPanelManager->AddPanel<ScriptEngineDebugPanel>(PanelCategory::View, "Script Engine", false);
		std::shared_ptr<StatisticsPanel> statisticsPanel = myPanelManager->AddPanel<StatisticsPanel>(PanelCategory::View, "Statistics", false);

		myPanelManager->Deserialize();

		const std::string& lastProjectPath = EditorSettings::Get().lastProjectPath;
		const bool openLastProject = EditorSettings::Get().loadLastOpenProject && !lastProjectPath.empty() && FileSystem::Exists(lastProjectPath);
		if (openLastProject)
		{
			OpenProject(lastProjectPath);
		}
		else if (FileSystem::Exists("DefaultProject/DefaultProject.eproj"))
		{
			OpenProject("DefaultProject/DefaultProject.eproj");
		}
		else
		{
			Application::Get().Close();
		}

		{
			EPOCH_PROFILE_SCOPE("Load fonts");

			UI::Fonts::Add("Regular", "Resources/Fonts/opensans/OpenSans-Regular.ttf", true);

			static const ImWchar staticFontAwesomeRanges[] = { EP_ICON_MIN, EP_ICON_MAX, 0 };
			UI::FontConfiguration fontAwesome;
			fontAwesome.fontName = "FontAwesome";
			fontAwesome.filePath = "Resources/Fonts/FontAwesome/fontawesome-webfont.ttf";
			fontAwesome.size = 16.0f;
			fontAwesome.glyphRanges = staticFontAwesomeRanges;
			fontAwesome.mergeWithLast = true;
			UI::Fonts::Add(fontAwesome);

			UI::Fonts::Add("Bold", "Resources/Fonts/opensans/OpenSans-Bold.ttf");
		}

		GradientEditor::Get().Init();

		JobSystem& js = Application::Get().GetJobSystem();
		js.WaitUntilDone();

		// All icons needs to be loaded as the scene renderer needs to be able to create frame buffers (without stalling)
		mySceneRenderer = std::make_shared<SceneRenderer>();
		myDebugRenderer = std::make_shared<DebugRenderer>();
		myDebugRenderer->Init(mySceneRenderer->GetExternalCompositingFramebuffer());
		mySceneRenderer->SetDebugRenderer(myDebugRenderer);

		statisticsPanel->SetSceneRenderer(mySceneRenderer);
		statisticsPanel->SetDebugRenderer(myDebugRenderer);

		// All icons needs to be loaded so that the content browser can map file extensions to one of them
		contentBrowserPanel->Init();
	}

	void EditorLayer::OnDetach()
	{
		EPOCH_PROFILE_FUNC();

		if (mySceneState != SceneState::Edit) OnSceneStop();

		if (Project::GetActive())
		{
			CloseProject();
		}

		EditorSettingsSerializer::SaveSettings();

		Log::ShutdownAppConsole();
	}

	void EditorLayer::OnUpdate()
	{
		EPOCH_PROFILE_FUNC();

		if (!myActiveScene) return;

		switch (mySceneState)
		{
		case SceneState::Edit:
		{
			myEditorCamera.SetActive(myAllowEditorCameraMovement);
			myEditorCamera.OnUpdate();
			myActiveScene->OnUpdateEditor();
			mySceneRenderer->SetScene(myActiveScene);
			myActiveScene->OnRenderEditor(mySceneRenderer, myEditorCamera);

			OnRenderOverlay();

			if (EditorSettings::Get().autosaveEnabled)
			{
				myTimeSinceLastSave += CU::Timer::GetDeltaTime();
				if (myTimeSinceLastSave >= EditorSettings::Get().autosaveIntervalSeconds)
				{
					SaveSceneAuto();
				}
			}
			break;
		}
		case SceneState::Simulate:
		{
			myEditorCamera.SetActive(myAllowEditorCameraMovement);
			myEditorCamera.OnUpdate();
			myActiveScene->OnUpdateEditor();
			mySceneRenderer->SetScene(myActiveScene);
			myActiveScene->OnRenderEditor(mySceneRenderer, myEditorCamera);
			break;
		}
		case SceneState::Play:
		{
			myActiveScene->OnUpdateRuntime();
			mySceneRenderer->SetScene(myActiveScene);
			myActiveScene->OnRenderRuntime(mySceneRenderer);
			
			OnRenderGizmos();

			auto sceneUpdateQueue = myPostSceneUpdateQueue;
			myPostSceneUpdateQueue.clear();
			for (auto& fn : sceneUpdateQueue)
			{
				fn();
			}

			break;
		}
		}

		if (ScriptEngine::ShouldReloadAppAssembly())
		{
			bool reload = true;

			if (mySceneState != SceneState::Edit)
			{
				if (EditorSettings::Get().reloadScriptAssemblyWhilePlaying == ReloadScriptAssemblyWhilePlaying::Stop)
				{
					OnSceneStop();
					reload = true;
				}
				else
				{
					reload = false;
				}
			}

			if (reload)
			{
				ScriptEngine::ReloadAppAssembly();
			}
		}

		if (Input::IsMouseButtonPressed(MouseButton::Right) && !myStartedCameraClickInViewport && myViewportFocused && myViewportHovered)
		{
			myStartedCameraClickInViewport = true;
		}

		if (!Input::IsMouseButtonDown(MouseButton::Right) && myStartedCameraClickInViewport)
		{
			myStartedCameraClickInViewport = false;
		}

		//////////////////////////////////////
		//TODO: Put in an OnEvent() function//
		//////////////////////////////////////


		if (ImGui::IsKeyPressed(ImGuiKey_F9))
		{
			myDisplayCurrentColorGradingLUT = !myDisplayCurrentColorGradingLUT;
		}

		if (mySceneState == SceneState::Edit && (!ImGui::IsMouseDown(ImGuiMouseButton_Right) && !ImGui::IsMouseDown(ImGuiMouseButton_Middle)))
		{
			//ViewportSelection();

			if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
			{
				if (ImGui::IsKeyPressed(ImGuiKey_O))
				{
					OpenScene();
				}

				if (ImGui::IsKeyPressed(ImGuiKey_N))
				{
					NewScene();
				}

				if (ImGui::IsKeyPressed(ImGuiKey_S))
				{
					if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
					{
						SaveSceneAs();
					}
					else
					{
						SaveScene();
					}
				}
			}

			//if (CU::InputHandler::GetKeyDown(KeyCode::CONTROL))
			//{
			//	if (CU::InputHandler::GetKeyPressed(KeyCode::O))
			//	{
			//		OpenScene();
			//		CU::InputHandler::Reset();
			//	}
			//
			//	if (CU::InputHandler::GetKeyPressed(KeyCode::N))
			//	{
			//		NewScene();
			//	}
			//
			//	if (CU::InputHandler::GetKeyPressed(KeyCode::S))
			//	{
			//		if (CU::InputHandler::GetKeyDown(KeyCode::SHIFT))
			//		{
			//			SaveSceneAs();
			//			CU::InputHandler::Reset();
			//		}
			//		else
			//		{
			//			SaveScene();
			//		}
			//	}
			//}

			if (myViewportHovered && !ImGui::IsAnyItemActive())
			{
				if (ImGui::IsKeyPressed(ImGuiKey_F))
				{
					if (SelectionManager::GetSelectionCount(SelectionContext::Entity) == 1)
					{
						Entity entity = myActiveScene->GetEntityWithUUID(SelectionManager::GetSelections(SelectionContext::Entity)[0]);
						myEditorCamera.Focus(entity.GetWorldSpaceTransform().GetTranslation());
					}
				}


				if (ImGui::IsKeyPressed(ImGuiKey_Q))
				{
					myGizmoOperation = GizmoOperation::None;
				}

				if (ImGui::IsKeyPressed(ImGuiKey_W))
				{
					myGizmoOperation = GizmoOperation::Translate;
				}

				if (ImGui::IsKeyPressed(ImGuiKey_E))
				{
					myGizmoOperation = GizmoOperation::Rotate;
				}

				if (ImGui::IsKeyPressed(ImGuiKey_R))
				{
					myGizmoOperation = GizmoOperation::Scale;
				}

				if (ImGui::IsKeyPressed(ImGuiKey_X))
				{
					const bool isLocal = EditorSettings::Get().axisOrientationMode == AxisOrientationMode::Local;
					EditorSettings::Get().axisOrientationMode = isLocal ? AxisOrientationMode::World : AxisOrientationMode::Local;
				}

				//if (!CU::InputHandler::GetKeyDown(KeyCode::RIGHTMBUTTON) && !CU::InputHandler::GetKeyDown(KeyCode::MIDDLEMBUTTON))
				//{
				//	if (CU::InputHandler::GetKeyPressed(KeyCode::Q))
				//	{
				//		myGizmoOperation = GizmoOperation::None;
				//	}
				//
				//	if (CU::InputHandler::GetKeyPressed(KeyCode::W))
				//	{
				//		myGizmoOperation = GizmoOperation::Translate;
				//	}
				//
				//	if (CU::InputHandler::GetKeyPressed(KeyCode::E))
				//	{
				//		myGizmoOperation = GizmoOperation::Rotate;
				//	}
				//
				//	if (CU::InputHandler::GetKeyPressed(KeyCode::R))
				//	{
				//		myGizmoOperation = GizmoOperation::Scale;
				//	}
				//
				//	if (CU::InputHandler::GetKeyPressed(KeyCode::X))
				//	{
				//		const bool isLocal = EditorSettings::Get().axisOrientationMode == AxisOrientationMode::Local;
				//		EditorSettings::Get().axisOrientationMode = isLocal ? AxisOrientationMode::World : AxisOrientationMode::Local;
				//	}
				//}
			}
		}

		if (UI::IsWindowFocused(std::format("{}  Viewport", EP_ICON_DESKTOP).c_str()) || UI::IsWindowFocused(std::format("{}  Scene Hierarchy", EP_ICON_LIST).c_str()))
		{
			if (ImGui::IsKeyPressed(ImGuiKey_Delete))
			{
				auto selection = SelectionManager::GetSelections(SelectionContext::Entity);

				for (auto entityID : selection)
				{
					auto entity = myActiveScene->TryGetEntityWithUUID(entityID);
					if (!entity) continue;
					myActiveScene->DestroyEntity(entity);
				}
			}

			if (!ImGui::IsMouseDown(ImGuiMouseButton_Right) && ImGui::IsKeyPressed(ImGuiKey_D) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
			{
				auto& selection = SelectionManager::GetSelections(SelectionContext::Entity);
				std::vector<UUID> newSelection(selection.size());

				for (size_t i = 0; i < selection.size(); i++)
				{
					newSelection[i] = myActiveScene->DuplicateEntity(myActiveScene->GetEntityWithUUID(selection[i])).GetUUID();
				}

				SelectionManager::DeselectAll(SelectionContext::Entity);
				for (UUID id : newSelection)
				{
					SelectionManager::Select(SelectionContext::Entity, id);
				}
			}
		}

		if (ImGui::IsKeyPressed(ImGuiKey_P))
		{
			if (ImGui::IsKeyDown(ImGuiKey_LeftAlt) && myViewportHovered)
			{
				if (mySceneState == SceneState::Play)
				{
					OnSceneStop();
				}
				else
				{
					OnScenePlay();
				}
			}
		}
	}

	void EditorLayer::OnImGuiRender()
	{
		EPOCH_PROFILE_FUNC();

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || (ImGui::IsMouseClicked(ImGuiMouseButton_Right)))
		{
			if (!myStartedCameraClickInViewport && mySceneState != SceneState::Play)
			{
				ImGui::FocusWindow(GImGui->HoveredWindow);
				Input::SetCursorMode(CursorMode::Normal);
			}
		}

		BeginDockspace();

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				{
					const bool projectOpen = !(!Project::GetActive());

					if (ImGui::MenuItem("Open Scene...", "Ctrl+O", false, projectOpen))
					{
						OpenScene();
					}

					if (ImGui::MenuItem("New Scene", "Ctrl+N", false, projectOpen))
					{
						NewScene();
					}

					if (ImGui::MenuItem("Save Scene", "Ctrl+S", false, projectOpen))
					{
						SaveScene();
					}

					if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S", false, projectOpen))
					{
						SaveSceneAs();
					}
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Open Project..."))
				{
					OpenProject();
				}

				if (ImGui::MenuItem("Create Project"))
				{
					staticCreateNewProj = true;
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Build"))
				{
					RuntimeBuilder builder;
					builder.Build();
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Exit"))
				{
					Application::Get().Close();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				for (auto& [id, panelData] : myPanelManager->GetPanels(PanelCategory::Edit))
				{
					auto nameParts = CU::SplitString(panelData.name, "/");
					RecursivePanelMenuItem(nameParts, 0, panelData.isOpen);
				}
				ImGui::Separator();

				const std::filesystem::path scriptSolutionPath = Project::GetScriptSolutionPath();
				if (ImGui::MenuItem("Open C# Solution", nullptr, nullptr, FileSystem::Exists(scriptSolutionPath)))
				{
					FileSystem::OpenExternally(scriptSolutionPath);
				}

				if (ImGui::BeginMenu("C# Project"))
				{
					if (ImGui::MenuItem("Regenerate Solution", nullptr, nullptr, FileSystem::Exists(scriptSolutionPath)))
					{
						ScriptBuilder::RegenerateScriptSolution(Project::GetProjectDirectory());
					}

					if (ImGui::MenuItem("Build Assembly", nullptr, nullptr, FileSystem::Exists(scriptSolutionPath)))
					{
						ScriptBuilder::BuildScriptAssembly();
					}

					if (ImGui::MenuItem("Reload Assembly"))
					{
						ScriptEngine::ReloadAppAssembly();
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				for (auto& [id, panelData] : myPanelManager->GetPanels(PanelCategory::View))
				{
					auto nameParts = CU::SplitString(panelData.name, "/");
					RecursivePanelMenuItem(nameParts, 0, panelData.isOpen);
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		if (staticCreateNewProj)
		{
			staticCreateNewProj = false;
			ImGui::OpenPopup("Create Project");
		}
		ShowCreateProjectPopup(); // TODO: Make a easier way to display popups

		ViewportPanel();
		myPanelManager->OnImGuiRender();
		//ImGui::ShowDemoWindow();

		if (ImGui::Begin("Scene Renderer (TEMP)"))
		{
			UI::BeginPropertyGrid();

			const char* drawModeStrings[] = { "Shaded", "Albedo", "Normals", "Ambient Occlusion", "Roughness", "Metalness", "World Position" };
			int currentDrawMode = (int)mySceneRenderer->GetDrawMode();
			if (UI::Property_Dropdown("Draw Mode", drawModeStrings, 7, currentDrawMode))
			{
				mySceneRenderer->SetDrawMode((DrawMode)currentDrawMode);
			}

			UI::EndPropertyGrid();

			ImGui::End();
		}

		EndDockspace();
	}

	void EditorLayer::BeginDockspace()
	{
		// Note: Switch this to true to enable dockspace
		static bool dockspaceOpen = true;
		static bool opt_fullscreen_persistent = true;
		bool opt_fullscreen = opt_fullscreen_persistent;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dock able into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		{
			window_flags |= ImGuiWindowFlags_NoBackground;
		}

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Editor", &dockspaceOpen, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
		{
			ImGui::PopStyleVar(2);
		}

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		//float minWinSizeX = style.WindowMinSize.x;
		//float minWinSizeY = style.WindowMinSize.y;
		style.WindowMinSize.x = 100.0f;
		style.WindowMinSize.y = 30.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		//style.WindowMinSize.x = minWinSizeX;
		//style.WindowMinSize.y = minWinSizeY;
	}

	void EditorLayer::EndDockspace()
	{
		ImGui::End();
	}

	void EditorLayer::BeginViewportDockspace()
	{
		// Note: Switch this to true to enable dockspace
		static bool dockspaceOpen = true;
		static bool opt_fullscreen_persistent = true;
		bool opt_fullscreen = opt_fullscreen_persistent;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dock able into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = /*ImGuiWindowFlags_MenuBar | */ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		{
			window_flags |= ImGuiWindowFlags_NoBackground;
		}

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Viewport", &dockspaceOpen, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
		{
			ImGui::PopStyleVar(2);
		}

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		//float minWinSizeX = style.WindowMinSize.x;
		//float minWinSizeY = style.WindowMinSize.y;
		style.WindowMinSize.x = 100.0f;
		style.WindowMinSize.y = 30.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyViewportDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		//style.WindowMinSize.x = minWinSizeX;
		//style.WindowMinSize.y = minWinSizeY;
	}

	void EditorLayer::EndViewportDockspace()
	{
		ImGui::End();
	}

	void EditorLayer::RecursivePanelMenuItem(const std::vector<std::string>& aNameParts, uint32_t aDepth, bool& aIsOpen)
	{
		if (aDepth < aNameParts.size() - 1)
		{
			if (ImGui::BeginMenu(aNameParts[aDepth].c_str()))
			{
				RecursivePanelMenuItem(aNameParts, ++aDepth, aIsOpen);
				ImGui::EndMenu();
			}
		}
		else
		{
			if (ImGui::MenuItem(aNameParts.back().c_str(), nullptr, &aIsOpen))
			{
				myPanelManager->Serialize();
			}
		}
	}

	std::pair<float, float> EditorLayer::GetMouseViewportCord() const
	{
		auto [mx, my] = ImGui::GetMousePos();
		mx -= myViewportBounds.min.x;
		my -= myViewportBounds.min.y;
		CU::Vector2f viewportSize = myViewportBounds.max - myViewportBounds.min;
		my = viewportSize.y - my;

		return { mx, my };
	}

	std::pair<float, float> EditorLayer::GetMouseViewportSpace() const
	{
		auto [mx, my] = GetMouseViewportCord();
		CU::Vector2f viewportSize = myViewportBounds.max - myViewportBounds.min;

		return { (mx / viewportSize.x) * 2.0f - 1.0f, (my / viewportSize.y) * 2.0f - 1.0f };
	}

	bool EditorLayer::MouseInViewport()
	{
		auto [mouseX, mouseY] = GetMouseViewportSpace();
		return (mouseX > -1.0f && mouseX < 1.0f && mouseY > -1.0f && mouseY < 1.0f);
	}

	//TODO: Get working
	void EditorLayer::ViewportSelection()
	{
		////if (!myViewportFocused) return;
		//if (ImGuizmo::IsOver()) return;
		//
		//if (ImGui::IsKeyPressed(ImGuiKey_MouseLeft))
		//{
		//	if (MouseInViewport())
		//	{
		//		ImGui::ClearActiveID();
		//
		//		auto [pX, pY] = GetMouseViewportCord();
		// 
		//		//int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
		//		Entity clickedEntity;/* = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_ActiveScene.get());*/
		//
		//		bool ctrlDown = CU::InputHandler::GetKeyDown(KeyCode::LCONTROL) || CU::InputHandler::GetKeyDown(KeyCode::RCONTROL);
		//		bool shiftDown = CU::InputHandler::GetKeyDown(KeyCode::LSHIFT) || CU::InputHandler::GetKeyDown(KeyCode::RSHIFT);
		//
		//		if (!ctrlDown)
		//		{
		//			SelectionManager::DeselectAll();
		//		}
		//
		//		if (clickedEntity)
		//		{
		//			SelectionManager::Select(SelectionContext::Entity, clickedEntity.GetUUID());
		//		}
		//	}
		//}
	}

	void EditorLayer::HandleAssetDrop()
	{
		if (mySceneState != SceneState::Edit) return;
		if (!ImGui::BeginDragDropTarget()) return;

		auto data = ImGui::AcceptDragDropPayload("asset_payload");
		if (data)
		{
			bool grabFocus = false;

			unsigned count = data->DataSize / sizeof(AssetHandle);

			for (unsigned i = 0; i < count; i++)
			{
				AssetHandle assetHandle = *(((AssetHandle*)data->Data) + i);
				const AssetMetadata& assetData = Project::GetEditorAssetManager()->GetMetadata(assetHandle);

				if (assetData.type == AssetType::Scene)
				{
					OpenScene(Project::GetAssetDirectory() / assetData.filePath);
					break;
				}

				std::shared_ptr<Asset> asset = AssetManager::GetAsset<Asset>(assetHandle);

				if (!asset)
				{
					continue;
				}

				if (asset->GetAssetType() == AssetType::Mesh)
				{
					std::shared_ptr<Mesh> mesh = std::static_pointer_cast<Mesh>(asset);
					Entity meshEntity = myActiveScene->InstantiateMesh(mesh);

					OnEntityCreated(meshEntity);

					SelectionManager::DeselectAll(SelectionContext::Entity);
					SelectionManager::Select(SelectionContext::Entity, meshEntity.GetUUID());

					grabFocus = true;
				}
				else if (asset->GetAssetType() == AssetType::Texture)
				{
					Entity spriteEntity = myActiveScene->CreateEntity(assetData.filePath.stem().string());
					spriteEntity.AddComponent<SpriteRendererComponent>(asset->GetHandle());

					OnEntityCreated(spriteEntity);

					SelectionManager::DeselectAll(SelectionContext::Entity);
					SelectionManager::Select(SelectionContext::Entity, spriteEntity.GetUUID());

					grabFocus = true;
				}
				else if (asset->GetAssetType() == AssetType::Prefab)
				{
					std::shared_ptr<Prefab> prefab = std::static_pointer_cast<Prefab>(asset);
					Entity prefabEntity = myActiveScene->Instantiate(prefab);

					if (!EditorSettings::Get().createEntitiesAtOrigin)
					{
						auto& tc = prefabEntity.GetComponent<TransformComponent>();
						tc.transform.SetTranslation(myEditorCamera.GetTransform().GetTranslation() + myEditorCamera.GetTransform().GetForward() * 500.0f);
					}
					else
					{
						auto& tc = prefabEntity.GetComponent<TransformComponent>();
						tc.transform.SetTranslation(CU::Vector3f::Zero);
					}

					SelectionManager::DeselectAll(SelectionContext::Entity);
					SelectionManager::Select(SelectionContext::Entity, prefabEntity.GetUUID());

					grabFocus = true;
				}
			}

			staticFocusOnViewport |= grabFocus;
		}

		ImGui::EndDragDropTarget();
	}

	void EditorLayer::ShowCreateProjectPopup()
	{
		if (ImGui::BeginPopupModal("Create Project", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			static char* projectNameBuffer = new char[255];
			static char* newProjectFilePathBuffer = new char[512];
			static bool inited = false;
			if (!inited)
			{
				inited = true;
				memset(projectNameBuffer, 0, 255);
				memset(newProjectFilePathBuffer, 0, 512);
			}

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 7));

			UI::Fonts::PushFont("Bold");
			std::string fullProjectPath = strlen(newProjectFilePathBuffer) > 0 ? std::string(newProjectFilePathBuffer) + "\\" + std::string(projectNameBuffer) : "";
			ImGui::Text("Full Project Path:");
			UI::Fonts::PopFont();
			ImGui::SameLine();
			ImGui::Text(fullProjectPath.c_str());

			ImGui::SetNextItemWidth(500);
			ImGui::InputTextWithHint("##new_project_name", "Project Name...", projectNameBuffer, 255);

			ImVec2 label_size = ImGui::CalcTextSize("...", NULL, true);
			auto& style = ImGui::GetStyle();
			ImVec2 button_size = ImGui::CalcItemSize(ImVec2(0, 0), label_size.x + style.FramePadding.x + style.ItemInnerSpacing.x, label_size.y + style.FramePadding.y);

			ImGui::SetNextItemWidth(500 - button_size.x - style.FramePadding.x - style.ItemInnerSpacing.x);
			ImGui::InputTextWithHint("##new_project_location", "Project Location...", newProjectFilePathBuffer, 512);

			ImGui::SameLine();

			if (ImGui::Button("..."))
			{
				std::string result = FileSystem::OpenFolderDialog().string();
				memcpy(newProjectFilePathBuffer, result.data(), result.length());
			}

			if (ImGui::Button("Create", ImVec2(75, 0)))
			{
				if (CU::EndsWith(newProjectFilePathBuffer, "\\"))
				{
					CreateProject(std::string(newProjectFilePathBuffer) + std::string(projectNameBuffer));
				}
				else
				{
					CreateProject(fullProjectPath);
				}

				memset(projectNameBuffer, 0, 255);
				memset(newProjectFilePathBuffer, 0, 512);
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel", ImVec2(75, 0)))
			{
				memset(projectNameBuffer, 0, 255);
				memset(newProjectFilePathBuffer, 0, 512);
				ImGui::CloseCurrentPopup();
			}

			ImGui::PopStyleVar();
			ImGui::EndPopup();
		}
	}

	void EditorLayer::OnRenderOverlay()
	{
		EPOCH_PROFILE_FUNC();

		if (EditorSettings::Get().gridEnabled)
		{
			CU::Vector3f gridRotation;
			switch (EditorSettings::Get().gridPlane)
			{
			case GridPlane::X:
				gridRotation = { 0.0f, 0.0f, 90.0f };
				break;
			case GridPlane::Z:
				gridRotation = { 90.0f, 0.0f, 0.0f };
				break;
			default:
				break;
			}

			myDebugRenderer->DrawGrid(CU::Vector3f::Zero, gridRotation, EditorSettings::Get().gridSize, EditorSettings::Get().gridOpacity);
		}

		if (SelectionManager::GetSelectionCount(SelectionContext::Entity) > 0)
		{
			for (UUID entityID : SelectionManager::GetSelections(SelectionContext::Entity))
			{
				Entity entity = myActiveScene->GetEntityWithUUID(entityID);

				if (entity.HasComponent<CameraComponent>())
				{

				}

				if (entity.HasComponent<DirectionalLightComponent>())
				{
					auto& dlc = entity.GetComponent<DirectionalLightComponent>();

					const CU::Transform transform = myActiveScene->GetWorldSpaceTransform(entity);
					myDebugRenderer->DrawWireCone(transform.GetTranslation(), transform.GetRotation(), 5 * CU::Math::ToRad, 25, dlc.color);
				}

				if (entity.HasComponent<SpotlightComponent>())
				{
					auto& slc = entity.GetComponent<SpotlightComponent>();

					const CU::Transform transform = myActiveScene->GetWorldSpaceTransform(entity);
					myDebugRenderer->DrawWireCone(transform.GetTranslation(), transform.GetRotation(), slc.innerSpotAngle, slc.range, slc.color);
					myDebugRenderer->DrawWireCone(transform.GetTranslation(), transform.GetRotation(), slc.outerSpotAngle, slc.range, slc.color);
				}

				if (entity.HasComponent<PointLightComponent>())
				{
					auto& plc = entity.GetComponent<PointLightComponent>();

					const CU::Transform transform = myActiveScene->GetWorldSpaceTransform(entity);
					myDebugRenderer->DrawWireSphere(transform.GetTranslation(), transform.GetRotation(), plc.range, plc.color);
				}


				if (entity.HasComponent<BoxColliderComponent>())
				{
					auto& bcc = entity.GetComponent<BoxColliderComponent>();

					const CU::Transform transform = myActiveScene->GetWorldSpaceTransform(entity);

					const CU::Vector3f pos = transform.GetTranslation() + (transform.GetRotationQuat().GetRotationMatrix3x3() * bcc.offset);
					myDebugRenderer->DrawWireBox(pos, transform.GetRotation(), bcc.halfSize * transform.GetScale(), CU::Color::Green);
				}

				if (entity.HasComponent<SphereColliderComponent>())
				{
					auto& scc = entity.GetComponent<SphereColliderComponent>();

					const CU::Transform transform = myActiveScene->GetWorldSpaceTransform(entity);

					const CU::Vector3f pos = transform.GetTranslation() + (transform.GetRotationQuat().GetRotationMatrix3x3() * scc.offset);
					float radius = CU::Math::Max(transform.GetScale().x, CU::Math::Max(transform.GetScale().y, transform.GetScale().z)) * scc.radius;
					myDebugRenderer->DrawWireSphere(pos, transform.GetRotation(), radius, CU::Color::Green);
				}

				if (entity.HasAny<CapsuleColliderComponent, CharacterControllerComponent>())
				{
					CU::Vector3f pos;
					float radius = 0;
					float height = 0;

					const CU::Transform transform = myActiveScene->GetWorldSpaceTransform(entity);

					if (entity.HasComponent<CapsuleColliderComponent>())
					{
						auto& ccc = entity.GetComponent<CapsuleColliderComponent>();
						pos = transform.GetTranslation() + (transform.GetRotationQuat().GetRotationMatrix3x3() * ccc.offset);
						radius = CU::Math::Max(transform.GetScale().x, transform.GetScale().z) * ccc.radius;
						height = ccc.height * transform.GetScale().y;
					}
					else
					{
						auto& ccc = entity.GetComponent<CharacterControllerComponent>();
						pos = transform.GetTranslation() + (transform.GetRotationQuat().GetRotationMatrix3x3() * ccc.offset);
						radius = CU::Math::Max(transform.GetScale().x, transform.GetScale().z) * ccc.radius;
						height = ccc.height * transform.GetScale().y;
					}

					myDebugRenderer->DrawWireCapsule(pos, transform.GetRotation(), radius, height, CU::Color::Green);
				}
			}
		}

		// Camera Components
		{
			auto view = myActiveScene->GetAllEntitiesWith<CameraComponent>();
			for (auto entityID : view)
			{
				Entity entity{ entityID, myActiveScene.get() };
				const CU::Vector3f pos = entity.GetWorldSpaceTransform().GetTranslation();
				myDebugRenderer->DrawBillboardedQuad(EditorResources::CameraIcon, pos, myEditorCamera.GetTransform().GetTranslation(), CU::Vector3f::Up, CU::Color::White, 0.5f);
			}
		}

		// Sky Light Components
		{
			auto view = myActiveScene->GetAllEntitiesWith<SkyLightComponent>();
			for (auto entityID : view)
			{
				Entity entity{ entityID, myActiveScene.get() };
				const CU::Vector3f pos = entity.GetWorldSpaceTransform().GetTranslation();
				myDebugRenderer->DrawBillboardedQuad(EditorResources::SkyLightIcon, pos, myEditorCamera.GetTransform().GetTranslation(), CU::Vector3f::Up, CU::Color::White, 0.5f);
			}
		}

		// Directional Light Components
		{
			auto view = myActiveScene->GetAllEntitiesWith<DirectionalLightComponent>();
			for (auto entityID : view)
			{
				Entity entity{ entityID, myActiveScene.get() };
				const auto& lc = entity.GetComponent<DirectionalLightComponent>();
				const CU::Vector3f pos = entity.GetWorldSpaceTransform().GetTranslation();
				myDebugRenderer->DrawBillboardedQuad(EditorResources::DirectionalLightIcon, pos, myEditorCamera.GetTransform().GetTranslation(), CU::Vector3f::Up, lc.color, 0.5f);
			}
		}

		// Spotlight Components
		{
			auto view = myActiveScene->GetAllEntitiesWith<SpotlightComponent>();
			for (auto entityID : view)
			{
				Entity entity{ entityID, myActiveScene.get() };
				const auto& lc = entity.GetComponent<SpotlightComponent>();
				const CU::Vector3f pos = entity.GetWorldSpaceTransform().GetTranslation();
				myDebugRenderer->DrawBillboardedQuad(EditorResources::SpotlightIcon, pos, myEditorCamera.GetTransform().GetTranslation(), CU::Vector3f::Up, lc.color, 0.5f);
			}
		}

		// Point Light Components
		{
			auto view = myActiveScene->GetAllEntitiesWith<PointLightComponent>();
			for (auto entityID : view)
			{
				Entity entity{ entityID, myActiveScene.get() };
				const auto& lc = entity.GetComponent<PointLightComponent>();
				const CU::Vector3f pos = entity.GetWorldSpaceTransform().GetTranslation();
				myDebugRenderer->DrawBillboardedQuad(EditorResources::PointLightIcon, pos, myEditorCamera.GetTransform().GetTranslation(), CU::Vector3f::Up, lc.color, 0.5f);
			}
		}

		// ParticleEmitter Components
		{
			auto view = myActiveScene->GetAllEntitiesWith<ParticleSystemComponent>();
			for (auto entityID : view)
			{
				Entity entity{ entityID, myActiveScene.get() };
			}
		}

		myDebugRenderer->Render(myEditorCamera.GetViewMatrix(), myEditorCamera.GetProjectionMatrix());
	}

	void EditorLayer::OnRenderGizmos()
	{
		Entity cameraEntity = myActiveScene->GetPrimaryCameraEntity();
		if (!cameraEntity) return;

		CU::Transform worlTrans = myActiveScene->GetWorldSpaceTransform(cameraEntity);
		const CU::Matrix4x4f cameraViewMatrix = worlTrans.GetMatrix().GetFastInverse();
		SceneCamera& camera = cameraEntity.GetComponent<CameraComponent>().camera;
		camera.SetViewportSize(myActiveScene->GetViewportWidth(), myActiveScene->GetViewportHeight());

		for (const auto& [entityID, entityInstance] : ScriptEngine::GetEntityInstances())
		{
			Entity entity = myActiveScene->TryGetEntityWithUUID(entityID);
			if (entity)
			{
				if (entity.IsActive() && ScriptEngine::IsEntityInstantiated(entity))
				{
					ScriptEngine::CallMethod(entityInstance, "OnDrawGizmos");
				}
			}
		}

		if (SelectionManager::GetSelectionCount(SelectionContext::Entity) > 0)
		{
			for (UUID entityID : SelectionManager::GetSelections(SelectionContext::Entity))
			{
				Entity entity = myActiveScene->GetEntityWithUUID(entityID);

				if (entity.IsActive() && entity.HasComponent<ScriptComponent>() && ScriptEngine::IsEntityInstantiated(entity))
				{
					ScriptEngine::CallMethod(ScriptEngine::GetEntityInstance(entityID), "OnDrawGizmosSelected");
				}
			}
		}
		
		myDebugRenderer->Render(cameraViewMatrix, camera.GetProjectionMatrix());
	}

	ImGuizmo::OPERATION EditorLayer::GetGizmoOperation()
	{
		switch (myGizmoOperation)
		{
		case Epoch::EditorLayer::GizmoOperation::Translate:	return ImGuizmo::OPERATION::TRANSLATE;
		case Epoch::EditorLayer::GizmoOperation::Rotate:	return ImGuizmo::OPERATION::ROTATE;
		case Epoch::EditorLayer::GizmoOperation::Scale:		return ImGuizmo::OPERATION::SCALE;
		default: return ImGuizmo::OPERATION::TRANSLATE;
		}
	}

	void EditorLayer::DrawGizmos()
	{
		EPOCH_PROFILE_FUNC();

		if (myGizmoOperation == GizmoOperation::None) return;

		const auto& selections = SelectionManager::GetSelections(SelectionContext::Entity);
		if (selections.empty()) return;

		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

		const bool snap = ImGui::IsKeyDown(ImGuiKey_LeftCtrl);

		const float snapValue = GetGizmoSnapValue();
		const float snapValues[3] = { snapValue, snapValue, snapValue };

		CU::Matrix4x4f projectionMatrix = myEditorCamera.GetProjectionMatrix();
		CU::Matrix4x4f viewMatrix = myEditorCamera.GetViewMatrix();

		ImGuizmo::OPERATION operation = GetGizmoOperation();
		ImGuizmo::MODE mode = (ImGuizmo::MODE)EditorSettings::Get().axisOrientationMode;

		if (selections.size() == 1)
		{
			Entity entity = myActiveScene->GetEntityWithUUID(SelectionManager::GetSelections(SelectionContext::Entity)[0]);
			CU::Transform& entityTransform = entity.Transform();
			CU::Matrix4x4f transform = myActiveScene->GetWorldSpaceTransformMatrix(entity);

			if (ImGuizmo::Manipulate(
				&viewMatrix[0],
				&projectionMatrix[0],
				operation,
				mode,
				&transform[0],
				nullptr,
				snap ? snapValues : nullptr))
			{
				Entity parent = myActiveScene->TryGetEntityWithUUID(entity.GetParentUUID());
				if (parent)
				{
					CU::Matrix4x4f parentTransform = myActiveScene->GetWorldSpaceTransformMatrix(parent);
					transform = transform * CU::Matrix4x4f::GetFastInverse(parentTransform);
				}

				CU::Vector3f translation, rotation, scale;
				transform.Decompose(translation, rotation, scale);

				switch (myGizmoOperation)
				{
				case EditorLayer::GizmoOperation::Translate:
				{
					const CU::Vector3f originalTranslation = entityTransform.GetTranslation();
					CU::Vector3f deltaTranslation = translation - originalTranslation;

					if (fabs(deltaTranslation.x) < CU::Math::Epsilon) deltaTranslation.x = 0.0f;
					if (fabs(deltaTranslation.y) < CU::Math::Epsilon) deltaTranslation.y = 0.0f;
					if (fabs(deltaTranslation.z) < CU::Math::Epsilon) deltaTranslation.z = 0.0f;

					entityTransform.Translate(deltaTranslation);
					break;
				}
				case EditorLayer::GizmoOperation::Rotate:
				{
					CU::Vector3f originalRotation = entityTransform.GetRotation();

					originalRotation.x = fmodf(originalRotation.x + CU::Math::Pi, CU::Math::Tau) - CU::Math::Pi;
					originalRotation.y = fmodf(originalRotation.y + CU::Math::Pi, CU::Math::Tau) - CU::Math::Pi;
					originalRotation.z = fmodf(originalRotation.z + CU::Math::Pi, CU::Math::Tau) - CU::Math::Pi;

					CU::Vector3f deltaRotation = rotation - originalRotation;

					if (fabs(deltaRotation.x) < CU::Math::Epsilon) deltaRotation.x = 0.0f;
					if (fabs(deltaRotation.y) < CU::Math::Epsilon) deltaRotation.y = 0.0f;
					if (fabs(deltaRotation.z) < CU::Math::Epsilon) deltaRotation.z = 0.0f;

					entityTransform.Rotate(deltaRotation);
					break;
				}
				case EditorLayer::GizmoOperation::Scale:
				{
					const CU::Vector3f originalScale = entityTransform.GetScale();
					CU::Vector3f deltaScale = scale - originalScale;

					if (fabs(deltaScale.x) < CU::Math::Epsilon) deltaScale.x = 0.0f;
					if (fabs(deltaScale.y) < CU::Math::Epsilon) deltaScale.y = 0.0f;
					if (fabs(deltaScale.z) < CU::Math::Epsilon) deltaScale.z = 0.0f;

					entityTransform.SetScale(originalScale + deltaScale);
					break;
				}
				}
			}
		}
		else
		{
			CU::Vector3f medianTranslation;
			CU::Vector3f medianRotation;
			CU::Vector3f medianScale;
			for (auto entityID : selections)
			{
				Entity entity = myActiveScene->GetEntityWithUUID(entityID);
				myActiveScene->ConvertToWorldSpace(entity); // Needed for the matrix to be relative, but fucks up if parent and child selected
				medianTranslation += entity.Transform().GetTranslation();
				medianRotation += entity.Transform().GetRotation();
				medianScale += entity.Transform().GetScale();
			}
			medianTranslation /= (float)selections.size();
			medianRotation /= (float)selections.size();
			medianScale /= (float)selections.size();

			CU::Matrix4x4f medianMatrix = CU::Transform(medianTranslation, medianRotation, medianScale).GetMatrix();
			CU::Matrix4x4f deltaMatrix = CU::Matrix4x4f::Identity;

			if (ImGuizmo::Manipulate(
				&viewMatrix[0],
				&projectionMatrix[0],
				operation,
				mode,
				&medianMatrix[0],
				&deltaMatrix[0],
				snap ? snapValues : nullptr))
			{
				switch (EditorSettings::Get().multiTransformTarget)
				{
				case TransformationTarget::MedianPoint:
				{
					for (auto entityID : selections)
					{
						Entity entity = myActiveScene->GetEntityWithUUID(entityID);
						TransformComponent& transform = entity.GetComponent<TransformComponent>();
						transform.transform.SetTransform(transform.transform.GetMatrix() * deltaMatrix);
					}

					break;
				}
				case TransformationTarget::IndividualOrigins:
				{
					CU::Vector3f deltaTranslation, deltaRotation, deltaScale;
					deltaMatrix.Decompose(deltaTranslation, deltaRotation, deltaScale);

					for (auto entityID : selections)
					{
						Entity entity = myActiveScene->GetEntityWithUUID(entityID);
						CU::Transform& transform = entity.Transform();

						switch (myGizmoOperation)
						{
						case EditorLayer::GizmoOperation::Translate:
						{
							if (fabs(deltaTranslation.x) < CU::Math::Epsilon) deltaTranslation.x = 0.0f;
							if (fabs(deltaTranslation.y) < CU::Math::Epsilon) deltaTranslation.y = 0.0f;
							if (fabs(deltaTranslation.z) < CU::Math::Epsilon) deltaTranslation.z = 0.0f;

							transform.Translate(deltaTranslation);
							break;
						}
						case EditorLayer::GizmoOperation::Rotate:
						{
							if (fabs(deltaRotation.x) < CU::Math::Epsilon) deltaRotation.x = 0.0f;
							if (fabs(deltaRotation.y) < CU::Math::Epsilon) deltaRotation.y = 0.0f;
							if (fabs(deltaRotation.z) < CU::Math::Epsilon) deltaRotation.z = 0.0f;

							transform.Rotate(deltaRotation);
							break;
						}
						case EditorLayer::GizmoOperation::Scale:
						{
							if (fabs(deltaScale.x) < CU::Math::Epsilon) deltaScale.x = 0.0f;
							if (fabs(deltaScale.y) < CU::Math::Epsilon) deltaScale.y = 0.0f;
							if (fabs(deltaScale.z) < CU::Math::Epsilon) deltaScale.z = 0.0f;

							const CU::Vector3f originalScale = transform.GetScale();
							transform.SetScale(originalScale * deltaScale);
							break;
						}
						}
					}

					break;
				}
				default:
					break;
				}
			}

			// Needed for the matrix to be relative, but fucks up if parent and child selected
			for (auto entityID : selections)
			{
				Entity entity = myActiveScene->GetEntityWithUUID(entityID);
				myActiveScene->ConvertToLocalSpace(entity);
			}
		}
	}

	float EditorLayer::GetGizmoSnapValue()
	{
		const auto& editorSettings = EditorSettings::Get();

		switch (myGizmoOperation)
		{
		case GizmoOperation::Translate: return editorSettings.translationSnapValue;
		case GizmoOperation::Rotate: return editorSettings.rotationSnapValue;
		case GizmoOperation::Scale: return editorSettings.scaleSnapValue;
		}

		return 0.0f;
	}

	static void ReplaceToken(std::string& outStr, const char* aToken, const std::string& aValue)
	{
		size_t pos = 0;
		while ((pos = outStr.find(aToken, pos)) != std::string::npos)
		{
			outStr.replace(pos, strlen(aToken), aValue);
			pos += strlen(aToken);
		}
	}

	void EditorLayer::CreateProject(const std::filesystem::path& aPath)
	{
		EPOCH_PROFILE_FUNC();

		if (!std::filesystem::exists(aPath))
		{
			std::filesystem::create_directories(aPath);
		}

		std::filesystem::copy("Resources/NewProjectTemplate", aPath, std::filesystem::copy_options::recursive);

		std::string projName = aPath.stem().string();

		// Premake File
		{
			std::ifstream stream(aPath / "premake5.lua");
			EPOCH_ASSERT(stream.is_open(), "Could not open projects premake file!");
			std::stringstream ss;
			ss << stream.rdbuf();
			stream.close();

			std::string str = ss.str();
			ReplaceToken(str, "$PROJECT_NAME$", projName);

			std::ofstream ostream(aPath / "premake5.lua");
			ostream << str;
			ostream.close();
		}

		// Project File
		{
			std::ifstream stream(aPath / "Project.eproj");
			EPOCH_ASSERT(stream.is_open(), "Could not open project file!");
			std::stringstream ss;
			ss << stream.rdbuf();
			stream.close();

			std::string str = ss.str();
			ReplaceToken(str, "$PROJECT_NAME$", projName);

			std::ofstream ostream(aPath / "Project.eproj");
			ostream << str;
			ostream.close();

			std::string newProjectFileName = std::string(projName) + ".eproj";
			std::filesystem::rename(aPath / "Project.eproj", aPath / newProjectFileName);
		}

		std::filesystem::create_directory(aPath / "Assets");
		std::filesystem::create_directory(aPath / "Regs");

		ScriptBuilder::RegenerateScriptSolution(aPath.string());
		OpenProject(aPath / (projName + ".eproj"));
	}

	void EditorLayer::SaveProject()
	{
		if (!Project::GetActive()) return;

		auto project = Project::GetActive();

		project->GetConfig().startScene = "";
		if (!myEditorScenePath.empty())
		{
			project->GetConfig().startScene = std::filesystem::relative(myEditorScenePath, Project::GetAssetDirectory());
		}

		project->GetConfig().editorCameraPosition = myEditorCamera.GetTransform().GetTranslation();
		project->GetConfig().editorCameraRotation = myEditorCamera.GetTransform().GetRotation();

		ProjectSerializer serializer(project);
		serializer.Serialize(project->GetProjectPath());
	}

	void EditorLayer::CloseProject()
	{
		SaveProject();

		SelectionManager::DeselectAll();
		ScriptEngine::SetSceneContext(nullptr, nullptr);

		myPanelManager->OnSceneChanged(nullptr);
		if (mySceneRenderer) mySceneRenderer->SetScene(nullptr);
		myActiveScene = nullptr;

		EPOCH_ASSERT(myEditorScene.use_count() == 1, "Scene will not be destroyed after project is closed - something is still holding scene refs!");
		myEditorScene = nullptr;

		Project::SetActive(nullptr);
	}

	bool EditorLayer::OpenProject()
	{
		std::filesystem::path filepath = FileSystem::OpenFileDialog({ { "Epoch project", "eproj" } });

		if (filepath.empty())
		{
			return false;
		}

		OpenProject(filepath);
		return true;
	}

	void EditorLayer::OpenProject(const std::filesystem::path& aPath)
	{
		EPOCH_PROFILE_FUNC();

		if (!std::filesystem::exists(aPath))
		{
			LOG_ERROR("Tried to open a project that doesn't exist. Project path: {}", aPath.string());
			return;
		}

		if (Project::GetActive())
		{
			CloseProject();
		}

		std::shared_ptr<Project> project = std::make_shared<Project>();
		ProjectSerializer serializer(project);
		serializer.Deserialize(aPath);
		Project::SetActive(project);

		auto appAssemblyPath = Project::GetScriptModuleFilePath();
		if (!appAssemblyPath.empty())
		{
			ScriptBuilder::RegenerateScriptSolution(Project::GetProjectDirectory());
			ScriptBuilder::BuildScriptAssembly();

			ScriptEngine::OnProjectChanged(project);
			ScriptEngine::LoadAppAssembly();
		}

		std::filesystem::path assetDirectory = Project::GetAssetDirectory();
		if (!std::filesystem::exists(assetDirectory)) FileSystem::CreateDirectory(assetDirectory);

		myPanelManager->OnProjectChanged(project);

		bool hasStartScene = !project->GetConfig().startScene.empty();
		if (hasStartScene && FileSystem::Exists(assetDirectory / project->GetConfig().startScene))
		{
			hasStartScene = OpenScene((assetDirectory / project->GetConfig().startScene).string());
		}
		else
		{
			NewScene();
		}

		myEditorCamera.GetTransform().SetTranslation(project->GetConfig().editorCameraPosition);
		myEditorCamera.GetTransform().SetRotation(project->GetConfig().editorCameraRotation);

		EditorSettings::Get().lastProjectPath = Project::GetProjectPath().string();
		EditorSettingsSerializer::SaveSettings();
	}

	void EditorLayer::UpdateWindowTitle(const std::string& aSceneName)
	{
		const std::string title = std::format("{} - {} - Epoch <{}>", aSceneName, Project::GetActive()->GetConfig().name, "DX11");
		Application::Get().GetWindow().SetTitle(title);
	}

	void EditorLayer::OpenScene()
	{
		std::filesystem::path filepath = FileSystem::OpenFileDialog({ { "Epoch Scene", "epoch" } }, Project::GetAssetDirectory().string().c_str());

		if (!filepath.empty())
		{
			OpenScene(filepath);
		}
	}

	bool EditorLayer::OpenScene(const std::filesystem::path& aPath, const bool aCheckAutoSave)
	{
		EPOCH_PROFILE_FUNC();

		if (!std::filesystem::exists(aPath))
		{
			LOG_ERROR("Tried loading a non-existing scene: {}", aPath.string());
			return false;
		}

		if (aPath.extension().string() != ".epoch")
		{
			LOG_ERROR("Could not load {} - not a scene file.", aPath.filename().string());
			return false;
		}


		std::filesystem::path scenePath = aPath;
		if (aCheckAutoSave)
		{
			std::string autoSavePath = (Project::GetAutosaveDirectory() / aPath.stem()).string() + ".auto";
			if (FileSystem::Exists(autoSavePath) && FileSystem::IsNewer(autoSavePath, aPath))
			{
				scenePath = std::filesystem::path(autoSavePath);
			}
		}

		auto assetManager = Project::GetEditorAssetManager();
		const auto& metadata = assetManager->GetMetadata(assetManager->GetAssetHandleFromFilePath(aPath));

		std::shared_ptr<Scene> newScene = std::make_shared<Scene>(metadata.handle);
		SceneSerializer serializer(newScene);

		if (!serializer.Deserialize(scenePath))
		{
			return false;
		}

		if (mySceneState != SceneState::Edit)
		{
			OnSceneStop();
		}

		SelectionManager::DeselectAll(SelectionContext::Entity);
		ScriptEngine::SetSceneContext(nullptr, nullptr);

		UpdateWindowTitle(aPath.stem().string());

		myEditorScene = newScene;
		myEditorScenePath = aPath;

		myActiveScene = myEditorScene;
		myPanelManager->OnSceneChanged(myActiveScene);
		if (mySceneRenderer) mySceneRenderer->SetScene(nullptr);
		ScriptEngine::SetSceneContext(myActiveScene, mySceneRenderer);

		myTimeSinceLastSave = 0.0f;

		return true;
	}

	void EditorLayer::NewScene(const std::string& aName)
	{
		SelectionManager::DeselectAll(SelectionContext::Entity);
		ScriptEngine::SetSceneContext(nullptr, nullptr);

		UpdateWindowTitle(aName);

		myEditorScene = std::make_shared<Scene>(aName);
		myEditorScenePath = std::filesystem::path();

		myActiveScene = myEditorScene;
		myPanelManager->OnSceneChanged(myActiveScene);
		ScriptEngine::SetSceneContext(myActiveScene, mySceneRenderer);

		if (mySceneRenderer) mySceneRenderer->SetScene(nullptr);
	}

	void EditorLayer::SaveScene()
	{
		if (!myEditorScenePath.empty())
		{
			SerializeScene(myActiveScene, myEditorScenePath);
		}
		else
		{
			SaveSceneAs();
		}
	}

	void EditorLayer::SaveSceneAs()
	{
		if (!myActiveScene) return;

		std::filesystem::path filepath = FileSystem::SaveFileDialog({ { "Epoch Scene", "epoch" } }, Project::GetAssetDirectory().string().c_str());

		if (filepath.empty()) return;

		SerializeScene(myActiveScene, filepath);
		myEditorScenePath = filepath;
		UpdateWindowTitle(filepath.stem().string());

		auto assetManager = Project::GetEditorAssetManager();
		const auto& assetHandle = assetManager->ImportAsset(filepath);
		myActiveScene->SetAssetHandle(assetHandle);
	}

	void EditorLayer::SaveSceneAuto()
	{
		if (!myEditorScenePath.empty())
		{
			std::string autoSavePath = (Project::GetAutosaveDirectory() / myEditorScenePath.stem()).string() + ".auto";
			SerializeScene(myEditorScene, autoSavePath);
			LOG_INFO("Scene autosaved: {}", std::filesystem::relative(autoSavePath, Project::GetProjectDirectory()).string());
		}
	}

	void EditorLayer::SerializeScene(std::shared_ptr<Scene> aScene, const std::filesystem::path& aPath)
	{
		SceneSerializer serializer(aScene);
		serializer.Serialize(aPath.string());
		myTimeSinceLastSave = 0.0f;
	}

	void EditorLayer::QueueSceneTransition(AssetHandle aScene)
	{
		myPostSceneUpdateQueue.emplace_back([this, aScene]() { OnSceneTransition(aScene); });
	}

	void EditorLayer::ToolbarPanel()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto& colors = ImGui::GetStyle().Colors;
		const ImVec4& buttonHovered = colors[ImGuiCol_ButtonHovered];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
		const auto& buttonActive = colors[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

		ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse);

		bool toolbarEnabled = (bool)myActiveScene;

		ImVec4 tintColor = ImVec4(1, 1, 1, 1);
		if (!toolbarEnabled)
		{
			tintColor.w = 0.5f;
		}

		float size = ImGui::GetWindowHeight() - 4.0f;
		ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));


		bool hasPlayButton = mySceneState == SceneState::Edit || mySceneState == SceneState::Play;
		//bool hasSimulateButton = mySceneState == SceneState::Edit;
		bool hasPauseButton = mySceneState != SceneState::Edit;

		if (hasPlayButton)
		{
			auto icon = (mySceneState == SceneState::Edit) ? EditorResources::PlayButton : EditorResources::StopButton;
			if (ImGui::ImageButton((ImTextureID)icon->GetView(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
			{
				if (mySceneState == SceneState::Edit)
				{
					OnScenePlay();
					staticFocusOnViewport = true;
				}
				else if (mySceneState == SceneState::Play)
				{
					OnSceneStop();
				}
			}
			if (ImGui::BeginItemTooltip())
			{
				(mySceneState == SceneState::Edit) ? ImGui::Text(" Play ") : ImGui::Text(" Stop ");
				ImGui::EndTooltip();
			}
		}

		//if (hasSimulateButton)
		//{
		//	if (hasPlayButton)
		//	{
		//		ImGui::SameLine();
		//	}
		//
		//	auto icon = (mySceneState == SceneState::Edit) ? EditorResources::SimulateButton : EditorResources::StopButton;
		//	if (ImGui::ImageButton((ImTextureID)icon->GetView(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
		//	{
		//		if (mySceneState == SceneState::Edit)
		//		{
		//			OnSceneSimulate();
		//		}
		//		else if (mySceneState == SceneState::Edit)
		//		{
		//			OnSceneStop();
		//		}
		//
		//	}
		//	if (ImGui::BeginItemTooltip())
		//	{
		//		(mySceneState == SceneState::Edit) ? ImGui::Text(" Simulate ") : ImGui::Text(" Stop ");
		//		ImGui::EndTooltip();
		//	}
		//}

		if (hasPauseButton)
		{
			bool isPaused = myActiveScene->IsPaused();
			ImGui::SameLine();
			{
				auto icon = isPaused ? EditorResources::PlayButton : EditorResources::PauseButton;
				if (ImGui::ImageButton((ImTextureID)icon->GetView(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
				{
					myActiveScene->SetPaused(!isPaused);
					if (!isPaused)
					{
						staticFocusOnViewport = true;
					}
				}
				if (ImGui::BeginItemTooltip())
				{
					isPaused ? ImGui::Text(" Play ") : ImGui::Text(" Pause ");
					ImGui::EndTooltip();
				}
			}

			// Step button
			//if (isPaused)
			//{
			//	ImGui::SameLine();
			//	{
			//		auto icon = EditorResources::StepButton;
			//		ImGui::ImageButton((ImTextureID)icon->GetView(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled;
			//
			//		static float stepHeldTime = 0.0f;
			//		if (ImGui::IsItemActive())
			//		{
			//			stepHeldTime -= CU::Timer::GetDeltaTime();
			//			if (stepHeldTime <= 0.0f)
			//			{
			//				stepHeldTime = 0.1f;
			//				myActiveScene->Step(5);
			//			}
			//		}
			//
			//		if (ImGui::IsItemDeactivated())
			//		{
			//			stepHeldTime = 0.0f;
			//		}
			//
			//		if (ImGui::BeginItemTooltip())
			//		{
			//			ImGui::Text("Step");
			//			ImGui::EndTooltip();
			//		}
			//	}
			//}
		}

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
		ImGui::End();
	}

	void EditorLayer::ViewportPanel()
	{
		ToolbarPanel();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin(std::format("{}  Viewport", EP_ICON_DESKTOP).c_str());

		if (staticFocusOnViewport)
		{
			staticFocusOnViewport = false;
			ImGui::SetKeyboardFocusHere(-1);
		}

		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		myViewportBounds.min = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		myViewportBounds.max = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		myViewportHovered = ImGui::IsWindowHovered();
		myViewportFocused = ImGui::IsWindowFocused();

		myAllowEditorCameraMovement = (ImGui::IsMouseHoveringRect(myViewportBounds.min, myViewportBounds.max) && myViewportFocused) || myStartedCameraClickInViewport;

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

		myEditorCamera.SetViewportSize((unsigned)viewportPanelSize.x, (unsigned)viewportPanelSize.y);
		mySceneRenderer->SetViewportSize((unsigned)viewportPanelSize.x, (unsigned)viewportPanelSize.y);
		myDebugRenderer->SetViewportSize((unsigned)viewportPanelSize.x, (unsigned)viewportPanelSize.y);
		myActiveScene->SetViewportSize((unsigned)viewportPanelSize.x, (unsigned)viewportPanelSize.y);

		auto curPos = ImGui::GetCursorPos();

		ImGui::Image((ImTextureID)mySceneRenderer->GetFinalPassTexture()->GetView(), viewportPanelSize);

		if (myShowGizmos && mySceneState == SceneState::Edit)
		{
			DrawGizmos();
		}

		HandleAssetDrop();

		if (myDisplayCurrentColorGradingLUT)
		{
			AssetHandle lutHandle(0);

			auto entities = myActiveScene->GetAllEntitiesWith<VolumeComponent>();
			for (auto entityID : entities)
			{
				Entity entity = Entity(entityID, myActiveScene.get());
				if (!entity.IsActive()) continue;

				const auto& vc = entities.get<VolumeComponent>(entityID);
				if (!vc.isActive) continue;

				lutHandle = vc.colorGrading.lut;

				break;
			}

			ImGui::SetCursorPos(curPos);
			UI::ShiftCursor(3, 2);
			if (mySceneRenderer->ColorGrading() && lutHandle != 0)
			{
				auto lut = AssetManager::GetAsset<Texture>(lutHandle);
				ImGui::Image((ImTextureID)lut->GetView(), { 256, 16 });
			}
			else
			{
				ImGui::Image((ImTextureID)Renderer::GetDefaultColorGradingLut()->GetView(), { 256, 16 });
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void EditorLayer::OnScenePlay()
	{
		if (EditorSettings::Get().autoSaveSceneBeforePlay)
		{
			SaveSceneAuto();
		}

		mySceneState = SceneState::Play;

		myActiveScene = std::make_shared<Scene>();
		myEditorScene->CopyTo(myActiveScene);
		myActiveScene->SetSceneTransitionCallback([this](AssetHandle scene) { QueueSceneTransition(scene); });
		ScriptEngine::SetSceneContext(myActiveScene, mySceneRenderer);

		auto console = myPanelManager->GetPanel<EditorConsolePanel>("Console");
		console->OnScenePlay();

		myPanelManager->OnSceneChanged(myActiveScene);
		myActiveScene->OnRuntimeStart();
	}

	void EditorLayer::OnSceneSimulate()
	{
		if (EditorSettings::Get().autoSaveSceneBeforePlay)
		{
			SaveSceneAuto();
		}

		mySceneState = SceneState::Simulate;

		myActiveScene = std::make_shared<Scene>();
		myEditorScene->CopyTo(myActiveScene);
		ScriptEngine::SetSceneContext(myActiveScene, mySceneRenderer);

		myPanelManager->OnSceneChanged(myActiveScene);
		myActiveScene->OnSimulationStart();
	}

	void EditorLayer::OnSceneStop()
	{
		if (mySceneState == SceneState::Play)
		{
			myActiveScene->OnRuntimeStop();
		}
		else if (mySceneState == SceneState::Simulate)
		{
			myActiveScene->OnSimulationStop();
		}

		ScriptEngine::SetSceneContext(nullptr, nullptr);

		mySceneState = SceneState::Edit;
		myActiveScene = myEditorScene;

		auto selection = SelectionManager::GetSelections(SelectionContext::Entity);
		for (auto id : selection)
		{
			if (!myActiveScene->TryGetEntityWithUUID(id))
			{
				SelectionManager::Deselect(SelectionContext::Entity, id);
			}
		}

		ScriptEngine::SetSceneContext(myActiveScene, mySceneRenderer);
		myPanelManager->OnSceneChanged(myActiveScene);

		Input::SetCursorMode(CursorMode::Normal);
	}

	void EditorLayer::OnSceneTransition(AssetHandle aScene)
	{
		const auto& metadata = Project::GetEditorAssetManager()->GetMetadata(aScene);

		std::shared_ptr<Scene> newScene = std::make_shared<Scene>(metadata.handle);
		SceneSerializer serializer(newScene);

		std::filesystem::path scenePath = metadata.filePath;
		if (serializer.Deserialize((Project::GetAssetDirectory() / scenePath).string()))
		{
			const CU::Vector2f viewportSize = myViewportBounds.max - myViewportBounds.min;
			newScene->SetViewportSize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);

			myActiveScene->OnRuntimeStop();

			SelectionManager::DeselectAll();

			myActiveScene = newScene;
			myActiveScene->SetSceneTransitionCallback([this](AssetHandle scene) { QueueSceneTransition(scene); });
			ScriptEngine::SetSceneContext(myActiveScene, mySceneRenderer);
			myActiveScene->OnRuntimeStart();
			myPanelManager->OnSceneChanged(myActiveScene);
		}
		else
		{
			CONSOLE_LOG_ERROR("Could not deserialize scene {} {}", scenePath.string(), aScene);
		}
	}

	void EditorLayer::OnEntityCreated(Entity aEntity)
	{
		if (!EditorSettings::Get().createEntitiesAtOrigin)
		{
			const CU::Transform& cameraTransform = myEditorCamera.GetTransform();
			aEntity.Transform().SetTranslation(cameraTransform.GetTranslation() + cameraTransform.GetForward() * 500.0f);
		}
	}

	void EditorLayer::OnEntityDeleted(Entity aEntity)
	{
		SelectionManager::Deselect(SelectionContext::Entity, aEntity.GetUUID());
	}

	void EditorLayer::OnSetToEditorCameraTransform(Entity aEntity)
	{
		myActiveScene->ConvertToWorldSpace(aEntity);
		aEntity.Transform().SetTransform(myEditorCamera.GetTransform());
		myActiveScene->ConvertToLocalSpace(aEntity);
	}

	void EditorLayer::OnResetBoneTransforms(Entity aEntity)
	{
		if (aEntity.HasComponent<SkinnedMeshRendererComponent>())
		{
			auto& smrc = aEntity.GetComponent<SkinnedMeshRendererComponent>();
			auto meshAssset = AssetManager::GetAsset<Mesh>(smrc.mesh);

			if (!meshAssset->HasSkeleton())
			{
				return;
			}

			auto skeleton = meshAssset->GetSkeleton();

			for (uint32_t i = 0; i < skeleton->GetNumBones(); i++)
			{
				const Skeleton::Bone& bone = skeleton->GetBone(i);
				EPOCH_ASSERT(i < smrc.boneEntityIds.size(), "Missing bone!");
				auto boneEntity = myActiveScene->GetEntityWithUUID(smrc.boneEntityIds[i]);

				auto& tc = boneEntity.GetComponent<TransformComponent>();
				tc.transform.SetTranslation(bone.position);
				tc.transform.SetRotation(bone.orientation.GetEulerAngles());
				tc.transform.SetScale(bone.scale);
			}
		}
	}

	void EditorLayer::OnCurrentSceneRenamed(const AssetMetadata& aMetadata)
	{
		myEditorScenePath = Project::GetEditorAssetManager()->GetFileSystemPath(aMetadata);
		myActiveScene->SetName(aMetadata.filePath.stem().string());
		UpdateWindowTitle(aMetadata.filePath.stem().string());
	}
}
