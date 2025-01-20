#pragma once
#include <string>
#include <GLFW/glfw3.h>
#ifdef PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>
#endif
#include "Events/Event.h"

namespace Epoch
{
	struct WindowProperties
	{
		std::string title = "Epoch Application";
		unsigned width = 1280;
		unsigned height = 720;
		std::string iconPath;
	};

	struct Resolution
	{
		uint32_t width;
		uint32_t height;
		uint32_t refreshRate;
	};

	class Window
	{
		using EventCallbackFn = std::function<void(Event&)>;

	public:
		Window() = delete;
		~Window();

		static Window* Create(const WindowProperties& aProps = WindowProperties());
		virtual void SetEventCallback(const EventCallbackFn& aCallback) { myData.eventCallback = aCallback; }

		void Update();

		uint32_t GetWidth() const { return myData.width; }
		uint32_t GetHeight() const { return myData.height; }

		void Maximize();
		void Minimize();
		void Restore();

		bool IsFullscreen() const;
		void SetFullscreen(bool aFullscreen = true);

		const std::string& GetTitle() const { return myData.title; }
		void SetTitle(const std::string& aTitle);

		std::vector<Resolution> GetVideoModes();

		inline void* GetNativeWindow() const { return myWindow; }
#ifdef PLATFORM_WINDOWS
		inline HWND GetWin32Window() const { return glfwGetWin32Window(myWindow); }
#endif

	private:
		Window(const WindowProperties& aProps);

		void Init(const WindowProperties& aProps);
		void Shutdown();

		GLFWmonitor* GetOverlappedMonitor();

	private:
		GLFWwindow* myWindow;

		struct WindowData
		{
			std::string title = "";
			uint32_t width = 0;
			uint32_t height = 0;

			EventCallbackFn eventCallback;
		};
		WindowData myData;

		int32_t myStoredSize[2];
		int32_t myStoredPosition[2];
	};
}
