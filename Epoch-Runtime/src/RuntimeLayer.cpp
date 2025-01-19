#include "RuntimeLayer.h"
#include <Epoch/Core/Application.h>
#include <Epoch/Core/Window.h>
#include <Epoch/Core/GraphicsEngine.h>
#include <Epoch/Rendering/RenderPipeline.h>
#include <Epoch/Rendering/Renderer.h>
#include <Epoch/Project/ProjectSerializer.h>
#include <Epoch/Assets/AssetPack/AssetPack.h>
#include <Epoch/Scene/SceneRenderer.h>
#include <Epoch/Scene/SceneSerializer.h>
#include <Epoch/Script/ScriptEngine.h>
#include <Epoch/Rendering/RHI.h>
#include <Epoch/Core/Input.h>
#include <Epoch/Debug/Instrumentor.h>
#include <d3d11.h>

namespace Epoch
{
	RuntimeLayer::RuntimeLayer(std::string_view aProjectPath) : myProjectPath(aProjectPath)
	{
	}

	void RuntimeLayer::OnAttach()
	{
		Log::InitAppConsole(false);

		mySceneRenderer = std::make_shared<SceneRenderer>();

		OpenProject();

		FramebufferSpecification specs;
		specs.swapChainTarget = true;
		
		PipelineSpecification pipelineSpecs("Runtime Copy");
		pipelineSpecs.targetFramebuffer = Framebuffer::Create(specs);
		pipelineSpecs.shader = Renderer::GetShaderLibrary()->Get("CopyTexture");
		myCompositePipeline = RenderPipeline::Create(pipelineSpecs);
		
		OnScenePlay();
	}

	void RuntimeLayer::OnDetach()
	{
		OnSceneStop();
		ScriptEngine::SetSceneContext(nullptr, nullptr);

		mySceneRenderer->SetScene(nullptr);
		myRuntimeScene = nullptr;

		Project::SetActive(nullptr);

		Log::ShutdownAppConsole();
	}
	
	void RuntimeLayer::OnUpdate()
	{
		Application& app = Application::Get();

		if (Input::IsKeyPressed(KeyCode::F11))
		{
			Window& window = app.GetWindow();

			if (window.IsFullscreen())
			{
				window.SetFullscreen(false);
			}
			else
			{
				window.SetFullscreen(true);
			}
		}

		if (Input::IsKeyPressed(KeyCode::F3))
		{
			LOG_INFO("F3");
			
			if (Input::IsKeyHeld(KeyCode::LeftAlt))
			{
				if (Instrumentor::Get().IsPaused())
				{
					LOG_INFO("Instrumentor unpause");
					Instrumentor::Get().Unpause();
				}
				else
				{
					LOG_INFO("Instrumentor pause");
					Instrumentor::Get().Pause();
				}
			}
			else
			{
				//Show debug data
			}
		}

		uint32_t width = app.GetWindowWidth();
		uint32_t height = app.GetWindowHeight();

		if (myViewportWidth != width || myViewportHeight != height)
		{
			myViewportWidth = width;
			myViewportHeight = height;

			mySceneRenderer->SetViewportSize(myViewportWidth, myViewportHeight);
			myRuntimeScene->SetViewportSize(myViewportWidth, myViewportHeight);
		}

		myRuntimeScene->OnUpdateRuntime();
		myRuntimeScene->OnRenderRuntime(mySceneRenderer);


		//Render (copy scene texture to back buffer)
		{
			auto finalImage = mySceneRenderer->GetFinalPassTexture();

			std::vector<ID3D11ShaderResourceView*> SRVs(1);
			SRVs.reserve(1);
			SRVs[0] = (ID3D11ShaderResourceView*)finalImage->GetView();

			RHI::GetContext()->PSSetShaderResources(0, 1, SRVs.data());

			if (finalImage)
			{
				Renderer::SetRenderPipeline(myCompositePipeline);
				Renderer::RenderQuad();
				Renderer::RemoveRenderPipeline(myCompositePipeline);
			}

			std::vector<ID3D11ShaderResourceView*> emptySRVs(1);
			RHI::GetContext()->PSSetShaderResources(0, 1, emptySRVs.data());
		}


		auto sceneUpdateQueue = myPostSceneUpdateQueue;
		myPostSceneUpdateQueue.clear();
		for (auto& fn : sceneUpdateQueue)
		{
			fn();
		}
	}

	void RuntimeLayer::OpenProject()
	{
		std::shared_ptr<Project> project = std::make_shared<Project>();
		ProjectSerializer serializer(project);
		serializer.DeserializeRuntime(myProjectPath);

		// Load asset pack
		myAssetPack = AssetPack::Load(Project::GetProjectDirectory() / project->GetConfig().assetDirectory / "AssetPack.eap");
		Project::SetActiveRuntime(project, myAssetPack);

		// Load app binary
		Buffer appBinary = myAssetPack->ReadAppBinary();
		ScriptEngine::LoadAppAssemblyRuntime(appBinary);
		appBinary.Release();

		LoadScene(Project::GetActive()->GetConfig().startScene);
	}

	void RuntimeLayer::LoadScene(uint64_t aSceneHandle)
	{
		std::shared_ptr<Scene> newScene = std::make_shared<Scene>(aSceneHandle);
		SceneSerializer serializer(newScene);
		serializer.Deserialize((Project::GetAssetDirectory() / Project::GetEditorAssetManager()->GetMetadata(aSceneHandle).filePath).string());
		myRuntimeScene = newScene;
		myRuntimeScene->SetViewportSize(myViewportWidth, myViewportHeight);
	}

	void RuntimeLayer::OnScenePlay()
	{
		myRuntimeScene->SetSceneTransitionCallback([this](AssetHandle handle) { QueueSceneTransition(handle); });
		mySceneRenderer->SetScene(myRuntimeScene);
		ScriptEngine::SetSceneContext(myRuntimeScene, mySceneRenderer);
		myRuntimeScene->OnRuntimeStart();
	}

	void RuntimeLayer::OnSceneStop()
	{
		myRuntimeScene->OnRuntimeStop();
	}

	void RuntimeLayer::QueueSceneTransition(uint64_t aScene)
	{
		myPostSceneUpdateQueue.push_back([this, aScene]()
		{
			myRuntimeScene->OnRuntimeStop();
			LoadScene(aScene);
			OnScenePlay();
		});
	}
}