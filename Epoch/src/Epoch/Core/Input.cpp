#include "epch.h"
#include "Input.h"
#include "Window.h"
#include "Epoch/Core/Application.h"
#include "Epoch/ImGui/ImGui.h"
#include <GLFW/glfw3.h>
#include <imgui/imgui_internal.h>


namespace Epoch
{
	void Input::Update()
	{
		// Cleanup disconnected controller
		for (auto it = staticControllers.begin(); it != staticControllers.end(); )
		{
			int id = it->first;
			if (glfwJoystickPresent(id) != GLFW_TRUE)
			{
				it = staticControllers.erase(it);
			}
			else
			{
				it++;
			}
		}

		// Update controllers
		for (int id = GLFW_JOYSTICK_1; id < GLFW_JOYSTICK_LAST; id++)
		{
			if (glfwJoystickPresent(id) == GLFW_TRUE)
			{
				if (!IsControllerPresent(id))
				{
					Controller& controller = staticControllers[id];
					controller.deadZones[0] = 0.1f;
					controller.deadZones[1] = 0.1f;
					controller.deadZones[2] = 0.1f;
					controller.deadZones[3] = 0.1f;
				}

				Controller& controller = staticControllers[id];
				controller.id = id;
				controller.name = glfwGetJoystickName(id);

				int buttonCount;
				const unsigned char* buttons = glfwGetJoystickButtons(id, &buttonCount);
				for (int i = 0; i < buttonCount; i++)
				{
					GamepadButton button = (GamepadButton)i;

					if (buttons[i] == GLFW_PRESS &&
						!(controller.buttonStates[button].state == KeyState::Pressed || controller.buttonStates[button].state == KeyState::Held))
					{
						controller.buttonStates[button].state = KeyState::Pressed;
					}
					else if (buttons[i] == GLFW_RELEASE &&
						(controller.buttonStates[button].state == KeyState::Pressed || controller.buttonStates[button].state == KeyState::Held))
					{
						controller.buttonStates[button].state = KeyState::Released;
					}
				}

				int axisCount;
				const float* axes = glfwGetJoystickAxes(id, &axisCount);
				for (int i = 0; i < axisCount; i++)
				{
					controller.axisStates[i] = abs(axes[i]) > controller.deadZones[i] ? axes[i] : 0.0f;
				}

				int hatCount;
				const unsigned char* hats = glfwGetJoystickHats(id, &hatCount);
				for (int i = 0; i < hatCount; i++)
				{
					controller.hatStates[i] = hats[i];
				}
				
				//Left & right trigger press and release (has to be done manually)
				{
					auto updateButtonState = [&](GamepadAxis aAxis, GamepadButton aButton)
						{
							controller.axisStates[(int)aAxis] = 
							CU::Math::Remap01(controller.axisStates[(int)aAxis], -1.0f, 1.0f);

							if (controller.axisStates[(int)aAxis] > 0.0f &&
								controller.buttonStates[aButton].state == KeyState::None)
							{
								controller.buttonStates[aButton].state = KeyState::Pressed;
							}

							if (controller.axisStates[(int)aAxis] <= 0.01f &&
								controller.buttonStates[aButton].state == KeyState::Held)
							{
								controller.buttonStates[aButton].state = KeyState::Released;
							}
						};

					updateButtonState(GamepadAxis::LeftTrigger, GamepadButton::LeftTrigger);
					updateButtonState(GamepadAxis::RightTrigger, GamepadButton::RightTrigger);
				}
			}
		}

		// Update mouse delta
		auto [mouseX, mouseY] = GetMousePosition();
		const CU::Vector2f mousePos(mouseX, mouseY);
		staticMouseDelta = (staticLastMousePos - mousePos);
		staticLastMousePos = mousePos;
	}

	bool Input::IsKeyPressed(KeyCode aKeyCode)
	{
		return staticKeyData.find(aKeyCode) != staticKeyData.end() && staticKeyData[aKeyCode].state == KeyState::Pressed;
	}

	bool Input::IsKeyHeld(KeyCode aKeyCode)
	{
		if (auto it = staticKeyData.find(aKeyCode); it != staticKeyData.end())
		{
			if (it->second.state == KeyState::Held || it->second.state == KeyState::Pressed)
			{
				return true;
			}
		}
		return false;

		//return staticKeyData.find(aKeyCode) != staticKeyData.end() && staticKeyData[aKeyCode].state == KeyState::Held;
	}

	bool Input::IsKeyReleased(KeyCode aKeyCode)
	{
		return staticKeyData.find(aKeyCode) != staticKeyData.end() && staticKeyData[aKeyCode].state == KeyState::Released;
	}

	bool Input::IsMouseButtonPressed(MouseButton aButton)
	{
		return staticMouseData.find(aButton) != staticMouseData.end() && staticMouseData[aButton].state == KeyState::Pressed;
	}

	bool Input::IsMouseButtonHeld(MouseButton aButton)
	{
		if (auto it = staticMouseData.find(aButton); it != staticMouseData.end())
		{
			if (it->second.state == KeyState::Held || it->second.state == KeyState::Pressed)
			{
				return true;
			}
		}
		return false;

		//return staticMouseData.find(aButton) != staticMouseData.end() && staticMouseData[aButton].state == KeyState::Held;
	}

	bool Input::IsMouseButtonReleased(MouseButton aButton)
	{
		return staticMouseData.find(aButton) != staticMouseData.end() && staticMouseData[aButton].state == KeyState::Released;
	}

	float Input::GetMouseX()
	{
		auto [x, y] = GetMousePosition();
		return (float)x;
	}

	float Input::GetMouseY()
	{
		auto [x, y] = GetMousePosition();
		return (float)y;
	}

	std::pair<float, float> Input::GetMousePosition()
	{
		auto& window = static_cast<Window&>(Application::Get().GetWindow());

		double x, y;
		glfwGetCursorPos(static_cast<GLFWwindow*>(window.GetNativeWindow()), &x, &y);
		return { (float)x, (float)y };
	}

	void Input::SetCursorMode(CursorMode aMode)
	{
		auto& window = static_cast<Window&>(Application::Get().GetWindow());
		glfwSetInputMode(static_cast<GLFWwindow*>(window.GetNativeWindow()), GLFW_CURSOR, GLFW_CURSOR_NORMAL + (int)aMode);

		if (Application::Get().GetSpecification().enableImGui)
		{
			UI::SetInputEnabled(aMode == CursorMode::Normal);
		}
	}

	CursorMode Input::GetCursorMode()
	{
		auto& window = static_cast<Window&>(Application::Get().GetWindow());
		return (CursorMode)(glfwGetInputMode(static_cast<GLFWwindow*>(window.GetNativeWindow()), GLFW_CURSOR) - GLFW_CURSOR_NORMAL);
	}

	bool Input::IsControllerPresent(int aId)
	{
		return staticControllers.find(aId) != staticControllers.end();
	}

	std::vector<int> Input::GetConnectedControllerIDs()
	{
		std::vector<int> ids;
		ids.reserve(staticControllers.size());
		for (auto [id, controller] : staticControllers)
		{
			ids.emplace_back(id);
		}

		return ids;
	}

	const Controller* Input::GetController(int aId)
	{
		if (!Input::IsControllerPresent(aId))
		{
			return nullptr;
		}

		return &staticControllers.at(aId);
	}

	std::string_view Input::GetControllerName(int aId)
	{
		if (!Input::IsControllerPresent(aId))
		{
			return {};
		}

		return staticControllers.at(aId).name;
	}

	bool Input::IsControllerButtonPressed(int aControllerID, GamepadButton aButton)
	{
		if (!Input::IsControllerPresent(aControllerID))
		{
			return false;
		}

		auto& contoller = staticControllers.at(aControllerID);
		return contoller.buttonStates.find(aButton) != contoller.buttonStates.end() && contoller.buttonStates[aButton].state == KeyState::Pressed;
	}

	bool Input::IsControllerButtonHeld(int aControllerID, GamepadButton aButton)
	{
		if (!Input::IsControllerPresent(aControllerID))
		{
			return false;
		}

		auto& contoller = staticControllers.at(aControllerID);
		
		if (auto it = contoller.buttonStates.find(aButton); it != contoller.buttonStates.end())
		{
			if (it->second.state == KeyState::Held || it->second.state == KeyState::Pressed)
			{
				return true;
			}
		}
		return false;
		
		//return contoller.buttonStates.find(aButton) != contoller.buttonStates.end() && contoller.buttonStates[aButton].state == KeyState::Held;
	}

	bool Input::IsControllerButtonReleased(int aControllerID, GamepadButton aButton)
	{
		if (!Input::IsControllerPresent(aControllerID))
		{
			return false;
		}

		auto& contoller = staticControllers.at(aControllerID);
		return contoller.buttonStates.find(aButton) != contoller.buttonStates.end() && contoller.buttonStates[aButton].state == KeyState::Released;
	}

	float Input::GetControllerAxis(int aControllerID, GamepadAxis aAxis)
	{
		if (!Input::IsControllerPresent(aControllerID))
		{
			return 0.0f;
		}

		int axis = (int)aAxis;
		const Controller& controller = staticControllers.at(aControllerID);
		if (controller.axisStates.find(axis) == controller.axisStates.end())
		{
			return 0.0f;
		}

		return controller.axisStates.at(axis);
	}

	uint8_t Input::GetControllerHat(int aControllerID, int aHat)
	{
		if (!Input::IsControllerPresent(aControllerID))
		{
			return 0;
		}

		const Controller& controller = staticControllers.at(aControllerID);
		if (controller.hatStates.find(aHat) == controller.hatStates.end())
		{
			return 0;
		}

		return controller.hatStates.at(aHat);
	}

	float Input::GetControllerDeadzone(int aControllerID, GamepadAxis aAxis)
	{
		if (!Input::IsControllerPresent(aControllerID))
		{
			return 0.0f;
		}

		const Controller& controller = staticControllers.at(aControllerID);
		return controller.deadZones.at((int)aAxis);
	}

	void Input::SetControllerDeadzone(int aControllerID, GamepadAxis aAxis, float aDeadzone)
	{
		if (!Input::IsControllerPresent(aControllerID))
		{
			return;
		}

		Controller& controller = staticControllers.at(aControllerID);
		controller.deadZones[(int)aAxis] = aDeadzone;
	}

	void Input::TransitionPressedKeys()
	{
		for (const auto& [key, keyData] : staticKeyData)
		{
			if (keyData.state == KeyState::Pressed)
			{
				UpdateKeyState(key, KeyState::Held);
			}
		}
	}

	void Input::TransitionPressedButtons()
	{
		for (const auto& [button, buttonData] : staticMouseData)
		{
			if (buttonData.state == KeyState::Pressed)
			{
				UpdateButtonState(button, KeyState::Held);
			}
		}

		for (const auto& [id, controller] : staticControllers)
		{
			for (const auto& [button, buttonStates] : controller.buttonStates)
			{
				if (buttonStates.state == KeyState::Pressed)
				{
					UpdateControllerButtonState(id, button, KeyState::Held);
				}
			}
		}
	}

	void Input::UpdateKeyState(KeyCode aKey, KeyState aNewState)
	{
		auto& keyData = staticKeyData[aKey];
		keyData.key = aKey;
		keyData.lastState = keyData.state;
		keyData.state = aNewState;
	}

	void Input::UpdateButtonState(MouseButton aButton, KeyState aNewState)
	{
		auto& mouseData = staticMouseData[aButton];
		mouseData.button = aButton;
		mouseData.lastState = mouseData.state;
		mouseData.state = aNewState;
	}

	void Input::UpdateControllerButtonState(int aController, GamepadButton aButton, KeyState aNewState)
	{
		auto& controllerButtonData = staticControllers.at(aController).buttonStates.at(aButton);
		controllerButtonData.button = aButton;
		controllerButtonData.lastState = controllerButtonData.state;
		controllerButtonData.state = aNewState;
	}

	void Input::ClearReleasedKeys()
	{
		for (const auto& [key, keyData] : staticKeyData)
		{
			if (keyData.state == KeyState::Released)
			{
				UpdateKeyState(key, KeyState::None);
			}
		}

		for (const auto& [button, buttonData] : staticMouseData)
		{
			if (buttonData.state == KeyState::Released)
			{
				UpdateButtonState(button, KeyState::None);
			}
		}

		for (const auto& [id, controller] : staticControllers)
		{
			for (const auto& [button, buttonStates] : controller.buttonStates)
			{
				if (buttonStates.state == KeyState::Released)
				{
					UpdateControllerButtonState(id, button, KeyState::None);
				}
			}
		}
	}

	void Input::SetScrollValues(const CU::Vector2f& aScrollValues)
	{
		staticMouseScroll = aScrollValues;
	}
}
