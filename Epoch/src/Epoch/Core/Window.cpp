#include "epch.h"
#include "Window.h"
#include "Epoch/Core/Application.h"
#include "Epoch/Core/Input.h"

namespace Epoch
{
	static bool staticGLFWInitialized = false;

	static void GLFWErrorCallback(int error, const char* description)
	{
		LOG_ERROR("GLFW Error ({}): {}", error, description);
	}

	Window::~Window()
	{
		EPOCH_PROFILE_FUNC();

		Shutdown();
	}

	Window* Window::Create(const WindowProperties& aProps)
	{
		return new Window(aProps);
	}

	void Window::Update()
	{
		EPOCH_PROFILE_FUNC();

		Input::SetScrollValues(CU::Vector2f::Zero);
		glfwPollEvents();
		Input::Update();
	}

	void Window::Maximize()
	{
		EPOCH_PROFILE_FUNC();

		glfwMaximizeWindow(myWindow);
	}

	void Window::Minimize()
	{
		EPOCH_PROFILE_FUNC();

		glfwIconifyWindow(myWindow);
	}

	void Window::Restore()
	{
		EPOCH_PROFILE_FUNC();

		glfwRestoreWindow(myWindow);
	}

	bool Window::IsFullscreen() const
	{
		return glfwGetWindowMonitor(myWindow) != nullptr;
	}

	void Window::SetFullscreen(bool aFullscreen)
	{
		EPOCH_PROFILE_FUNC();

		if (IsFullscreen() == aFullscreen) return;

		if (aFullscreen)
		{
			glfwGetWindowPos(myWindow, &myStoredPosition[0], &myStoredPosition[1]);
			glfwGetWindowSize(myWindow, &myStoredSize[0], &myStoredSize[1]);

			GLFWmonitor* monitor = GetOverlappedMonitor();// glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);

			glfwSetWindowMonitor(myWindow, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
		}
		else
		{
			glfwSetWindowMonitor(myWindow, nullptr, myStoredPosition[0], myStoredPosition[1], myStoredSize[0], myStoredSize[1], 0);
		}
	}

	void Window::SetTitle(const std::string& aTitle)
	{
		EPOCH_PROFILE_FUNC();

		myData.title = aTitle;
		glfwSetWindowTitle(myWindow, myData.title.c_str());
	}

	std::vector<Resolution> Window::GetVideoModes()
	{
		GLFWmonitor* monitor = GetOverlappedMonitor();

		int count;
		const GLFWvidmode* modes = glfwGetVideoModes(monitor, &count);

		std::vector<Resolution> resolutions(count);
		for (int i = 0; i < count; i++)
		{
			resolutions[i] = { (uint32_t)modes[i].width, (uint32_t)modes[i].height, (uint32_t)modes[i].refreshRate };
		}

		return resolutions;

		//int totalMonitor;
		//GLFWmonitor** monitors = glfwGetMonitors(&totalMonitor);
		//
		//printf("\n\n---------------------------------------------------------");
		//printf("\n Total monitor [%d]", totalMonitor);
		//
		//printf("\n primary monitor [%s]", glfwGetMonitorName(glfwGetPrimaryMonitor()));
		//printf("\n\n---------------------------------------------------------");
		//
		//for (int currMonitor = 0; currMonitor < totalMonitor; currMonitor++)
		//{
		//	printf("\n monitor name: [%s]", glfwGetMonitorName(monitors[currMonitor]));
		//
		//	int count;
		//	const GLFWvidmode* modes = glfwGetVideoModes(monitors[currMonitor], &count);
		//
		//	for (int i = 0; i < count; i++)
		//	{
		//		printf("\n  %d : [%d X %d]~[%d]", i, modes[i].width, modes[i].height, modes[i].refreshRate);
		//	}
		//
		//	printf("\n---------------------------------------------------------\n");
		//}
	}

	Window::Window(const WindowProperties& aProps)
	{
		Init(aProps);
	}

	void Window::Init(const WindowProperties& aProps)
	{
		EPOCH_PROFILE_FUNC();

		myData.title = aProps.title;
		myData.width = aProps.width;
		myData.height = aProps.height;

		if (!staticGLFWInitialized)
		{
			int success = glfwInit();
			EPOCH_ASSERT(success, "Could not intialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);

			staticGLFWInitialized = true;
		}

		{
			LOG_INFO("Creating Window: {} ({}, {})", myData.title, myData.width, myData.height);
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);
			myWindow = glfwCreateWindow((int)myData.width, (int)myData.height, myData.title.c_str(), nullptr, nullptr);
		}

		glfwSetWindowUserPointer(myWindow, &myData);

		bool isRawMouseMotionSupported = glfwRawMouseMotionSupported();
		if (isRawMouseMotionSupported)
		{
			glfwSetInputMode(myWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}
		else
		{
			LOG_WARNING("Raw mouse motion not supported.");
		}

		glfwSetWindowSizeCallback(myWindow, [](GLFWwindow* window, int width, int height)
			{
				auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

				//TEMP: TODO: Fix later with events
				Application::Get().OnWindowResized(width, height);
				data.width = width;
				data.height = height;
			});

		glfwSetWindowCloseCallback(myWindow, [](GLFWwindow* window)
			{
				//TEMP: TODO: Fix later with events
				Application::Get().Close();
			});

		glfwSetWindowIconifyCallback(myWindow, [](GLFWwindow* window, int iconified)
			{
				Application::Get().OnWindowMinimize((bool)iconified);
			});

		glfwSetDropCallback(myWindow, [](GLFWwindow* window, int path_count, const char* paths[])
			{
				//TODO: Add an event or something to handle this
			});

		glfwSetKeyCallback(myWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

				switch (action)
				{
				case GLFW_PRESS:
				{
					Input::UpdateKeyState((KeyCode)key, KeyState::Pressed);
					//KeyPressedEvent event((KeyCode)key, 0);
					//data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					Input::UpdateKeyState((KeyCode)key, KeyState::Released);
					//KeyReleasedEvent event((KeyCode)key);
					//data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					Input::UpdateKeyState((KeyCode)key, KeyState::Held);
					//KeyPressedEvent event((KeyCode)key, 1);
					//data.EventCallback(event);
					break;
				}
				}
			});

		glfwSetMouseButtonCallback(myWindow, [](GLFWwindow* window, int button, int action, int mods)
			{
				auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

				switch (action)
				{
				case GLFW_PRESS:
				{
					Input::UpdateButtonState((MouseButton)button, KeyState::Pressed);
					//MouseButtonPressedEvent event((MouseButton)button);
					//data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					Input::UpdateButtonState((MouseButton)button, KeyState::Released);
					//MouseButtonReleasedEvent event((MouseButton)button);
					//data.EventCallback(event);
					break;
				}
				}
			});

		glfwSetScrollCallback(myWindow, [](GLFWwindow* window, double xOffset, double yOffset)
			{
				auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

				Input::SetScrollValues({ (float)xOffset, (float)yOffset });
				//MouseScrolledEvent event((float)xOffset, (float)yOffset);
				//data.EventCallback(event);
			});

		glfwSetCursorPosCallback(myWindow, [](GLFWwindow* window, double x, double y)
			{
				auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

				//MouseMovedEvent event((float)x, (float)y);
				//data.EventCallback(event);
			});

		// Update window size to actual size
		{
			int width, height;
			glfwGetWindowSize(myWindow, &width, &height);
			myData.width = width;
			myData.height = height;
		}
	}

	void Window::Shutdown()
	{
		EPOCH_PROFILE_FUNC();

		glfwDestroyWindow(myWindow);
		glfwTerminate();
		staticGLFWInitialized = false;
	}

	GLFWmonitor* Window::GetOverlappedMonitor()
	{
		int nmonitors, i;
		int wx, wy, ww, wh;
		int mx, my, mw, mh;
		int overlap, bestoverlap;
		GLFWmonitor* bestmonitor;
		GLFWmonitor** monitors;
		const GLFWvidmode* mode;

		bestoverlap = 0;
		bestmonitor = NULL;

		glfwGetWindowPos(myWindow, &wx, &wy);
		glfwGetWindowSize(myWindow, &ww, &wh);
		monitors = glfwGetMonitors(&nmonitors);

		for (i = 0; i < nmonitors; i++)
		{
			mode = glfwGetVideoMode(monitors[i]);
			glfwGetMonitorPos(monitors[i], &mx, &my);
			mw = mode->width;
			mh = mode->height;

			overlap =
				CU::Math::Max(0, CU::Math::Min(wx + ww, mx + mw) - CU::Math::Max(wx, mx)) *
				CU::Math::Max(0, CU::Math::Min(wy + wh, my + mh) - CU::Math::Max(wy, my));

			if (bestoverlap < overlap)
			{
				bestoverlap = overlap;
				bestmonitor = monitors[i];
			}
		}

		return bestmonitor;
	}
}
