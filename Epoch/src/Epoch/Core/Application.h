#pragma once
#include <memory>
#include <string>
#include "LayerStack.h"
#include "JobSystem.h"
#include "Events/Event.h"
#include "Events/WindowEvent.h"
#include "Epoch/Script/ScriptEngineConfig.h"

int main(int argc, char** argv);

namespace Epoch
{
	class Window;
	class ImGuiLayer;

	struct ApplicationSpecification
	{
		bool isRuntime = false;
		std::string name = "Epoch Application";
		unsigned windowWidth = 1280;
		unsigned windowHeight = 720;
		bool startMaximized = false;
		bool startFullscreen = false;
		bool vSync = false;
		
		bool enableImGui = true;

		std::string cacheDirectory = "cache";

		ScriptEngineConfig scriptEngineConfig;
	};

	class Application
	{
	public:
		Application(ApplicationSpecification aAppSpec);
		virtual ~Application();

		static Application& Get() { return *myInstance; }
		const ApplicationSpecification& GetSpecification() const { return myApplicationSpecification; }

		Window& GetWindow() { return *myWindow; }
		uint32_t GetWindowWidth();
		uint32_t GetWindowHeight();

		JobSystem& GetJobSystem() { return myJobSystem; }

		void Run();
		void Close();

		virtual void OnEvent(Event& aEvent);

		void PushLayer(Layer* aLayer);
		void PushOverlay(Layer* aLayer);

		template<typename Func>
		void QueueEvent(Func&& func)
		{
			std::scoped_lock<std::mutex> lock(myEventQueueMutex);
			myEventQueue.emplace(func);
		}

		static bool IsRuntime() { return staticIsRuntime; }

	private:
		void ProcessEvents();

		bool OnWindowClose(const WindowCloseEvent& aEvent);
		bool OnWindowResize(const WindowResizeEvent& aEvent);
		bool OnWindowMinimize(const WindowMinimizeEvent& aEvent);

	private:
		ApplicationSpecification myApplicationSpecification;

		bool myIsRunning = true;
		bool myWindowMinimized = false;

		std::unique_ptr<Window> myWindow;

		LayerStack myLayerStack;
		ImGuiLayer* myImGuiLayer = nullptr;
		
		JobSystem myJobSystem;
		
		std::mutex myEventQueueMutex;
		std::queue<std::function<void()>> myEventQueue;

		inline static Application* myInstance = nullptr;

		friend int Main(int argc, char** argv);

	protected:
		static inline bool staticIsRuntime = false;
	};

	Application* CreateApplication();
}
