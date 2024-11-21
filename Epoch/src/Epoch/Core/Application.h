#pragma once
#include <memory>
#include <string>
#include "LayerStack.h"
#include "Epoch/Core/JobSystem.h"
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

		void PushLayer(Layer* aLayer);
		void PushOverlay(Layer* aLayer);

		void OnWindowResized(unsigned aWidth, unsigned aHeight);
		void OnWindowMinimize(bool aMinimized);

		static bool IsRuntime() { return staticIsRuntime; }

	private:
		void ProcessEvents();

	private:
		ApplicationSpecification myApplicationSpecification;

		bool myIsRunning = true;
		bool myWindowResized = false;
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
