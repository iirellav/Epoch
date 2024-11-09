#include "epch.h"
#include "Application.h"
#include <CommonUtilities/Timer.h>
#include <nfd.hpp>
#include "Epoch/Core/Window.h"
#include "Epoch/Core/Input.h"
#include "Epoch/Rendering/Font.h"
#include "Epoch/ImGui/ImGuiLayer.h"
#include "Epoch/Core/GraphicsEngine.h"
#include "Epoch/Rendering/Renderer.h"
#include "Epoch/Scene/SceneRenderer.h"
#include "Epoch/Physics/PhysicsSystem.h"
#include "Epoch/Script/ScriptEngine.h"

namespace Epoch
{
	Application::Application(ApplicationSpecification aAppSpec) : 
		myApplicationSpecification(aAppSpec)
	{
		EPOCH_PROFILE_FUNC();

		staticIsRuntime = aAppSpec.isRuntime;
		myInstance = this;

		myJobSystem.Init();

		WindowProperties windowProperties;
		windowProperties.title = aAppSpec.name;
		windowProperties.width = aAppSpec.windowWidth;
		windowProperties.height = aAppSpec.windowHeight;

		myWindow = std::unique_ptr<Window>(Window::Create(windowProperties));
		if (aAppSpec.startFullscreen) myWindow->SetFullscreen();
		else if (aAppSpec.startMaximized) myWindow->Maximize();

		if (NFD::Init() != NFD_OKAY)
		{
			EPOCH_ASSERT(false, "Failed to init NFD_Extended!");
			Close();
		}

		if (!GraphicsEngine::Get().Init())
		{
			EPOCH_ASSERT(false, "Failed to init the graphics engine!");
			Close();
		}

		GraphicsEngine::Get().GetVSyncBool() = aAppSpec.vSync;

		Renderer::Init();
		ScriptEngine::Init(aAppSpec.scriptEngineConfig);
		PhysicsSystem::Init();

		if (aAppSpec.enableImGui)
		{
			myImGuiLayer = new ImGuiLayer;
			PushOverlay(myImGuiLayer);
		}

		CU::Random::Init();
		CU::Timer::Init();
		Font::Init();
	}

	Application::~Application()
	{
		EPOCH_PROFILE_FUNC();

		LOG_INFO("Closing application");
		

		for (Layer* layer : myLayerStack)
		{
			layer->OnDetach();
			delete layer;
		}

		myJobSystem.WaitUntilDone();
		NFD::Quit();

		ScriptEngine::Shutdown();
		Font::Shutdown();
		Renderer::Shutdown();
	}

	void Application::Close()
	{
		myIsRunning = false;
	}

	void Application::PushLayer(Layer* aLayer)
	{
		myLayerStack.PushLayer(aLayer);
		aLayer->OnAttach();
	}

	void Application::PushOverlay(Layer* aLayer)
	{
		myLayerStack.PushOverlay(aLayer);
		aLayer->OnAttach();
	}

	void Application::OnWindowResized(unsigned aWidth, unsigned aHeight)
	{
		if (aWidth == 0 || aHeight == 0)
		{
			return;
		}

		myWindowResized = true;
	}

	void Application::OnWindowMinimize(bool aMinimized)
	{
		if (aMinimized)
		{
			LOG_DEBUG("Minimized");
		}
		else
		{
			LOG_DEBUG("Opened");
		}

		myWindowMinimized = aMinimized;
	}

	uint32_t Application::GetWindowWidth()
	{
		return myWindow->GetWidth();
	}

	uint32_t Application::GetWindowHeight()
	{
		return myWindow->GetHeight();
	}

	void Application::Run()
	{
		while (myIsRunning)
		{
			EPOCH_PROFILE_SCOPE("Frame");

			Input::TransitionPressedKeys();
			Input::TransitionPressedButtons();
			myWindow->Update();

			if (!myWindowMinimized)
			{
				GraphicsEngine::Get().BeginFrame();
				Renderer::BeginFrame();

				{
					EPOCH_PROFILE_SCOPE("Application::Run: Layer stack update");
					for (Layer* layer : myLayerStack)
					{
						layer->OnUpdate();
					}
				}

				GraphicsEngine::Get().RenderFrame();
				Renderer::RenderFrame();

				if (myApplicationSpecification.enableImGui)
				{
					myImGuiLayer->Begin();
					{
						EPOCH_PROFILE_SCOPE("Application::Run: Layer stack ImGui render");
						for (int i = 0; i < myLayerStack.Size(); i++)
						{
							myLayerStack[i]->OnImGuiRender();
						}
					}
					myImGuiLayer->End();
				}

				GraphicsEngine::Get().EndFrame();
				Renderer::EndFrame();
			}

			CU::Timer::Update();

			if (myWindowResized)
			{
				myWindowResized = false;
				GraphicsEngine::Get().OnWindowResize();
			}
			
			ProcessEvents();
		}
	}

	void Application::ProcessEvents()
	{
		ScriptEngine::InitializeRuntimeDuplicatedEntities();
		Input::ClearReleasedKeys();

		std::scoped_lock<std::mutex> lock(myEventQueueMutex);
		while (myEventQueue.size() > 0)
		{
			auto& func = myEventQueue.front();
			func();
			myEventQueue.pop();
		}
	}
}
