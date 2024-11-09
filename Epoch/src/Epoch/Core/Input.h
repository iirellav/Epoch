#pragma once
#include <map>
#include <vector>
#include "KeyCodes.h"

namespace Epoch
{
	struct ControllerButtonData
	{
		GamepadButton button;
		KeyState state = KeyState::None;
		KeyState lastState = KeyState::None;
	};

	struct Controller
	{
		int id;
		std::string name;
		std::map<GamepadButton, bool> buttonDown;
		std::map<GamepadButton, ControllerButtonData> buttonStates;
		std::map<int, float> axisStates;
		std::map<int, float> deadZones;
		std::map<int, uint8_t> hatStates;
	};

	struct KeyData
	{
		KeyCode key;
		KeyState state = KeyState::None;
		KeyState lastState = KeyState::None;
	};

	struct ButtonData
	{
		MouseButton button;
		KeyState state = KeyState::None;
		KeyState lastState = KeyState::None;
	};

	class Input
	{
	public:
		static bool IsKeyPressed(KeyCode aKeyCode);
		static bool IsKeyHeld(KeyCode aKeyCode);
		static bool IsKeyDown(KeyCode aKeyCode);
		static bool IsKeyReleased(KeyCode aKeyCode);

		static bool IsMouseButtonPressed(MouseButton aButton);
		static bool IsMouseButtonHeld(MouseButton aButton);
		static bool IsMouseButtonDown(MouseButton aButton);
		static bool IsMouseButtonReleased(MouseButton aButton);

		static float GetMouseX();
		static float GetMouseY();
		static std::pair<float, float> GetMousePosition();
		static CU::Vector2f GetMouseDelta() { return staticMouseDelta; }

		static CU::Vector2f GetMouseScroll() { return staticMouseScroll; }

		static void SetCursorMode(CursorMode aMode);
		static CursorMode GetCursorMode();

		// Controllers
		static bool IsControllerPresent(int aId);
		static std::vector<int> GetConnectedControllerIDs();
		static const Controller* GetController(int aId);
		static std::string_view GetControllerName(int aId);

		static bool IsControllerButtonPressed(int aControllerID, GamepadButton aButton);
		static bool IsControllerButtonHeld(int aControllerID, GamepadButton aButton);
		static bool IsControllerButtonDown(int aControllerID, GamepadButton aButton);
		static bool IsControllerButtonReleased(int aControllerID, GamepadButton aButton);
		
		static float GetControllerAxis(int aControllerID, GamepadAxis aAxis);
		static uint8_t GetControllerHat(int aControllerID, int aHat);

		static float GetControllerDeadzone(int aControllerID, GamepadAxis aAxis);
		static void SetControllerDeadzone(int aControllerID, GamepadAxis aAxis, float aDeadzone);
		
		static const std::map<int, Controller>& GetControllers() { return staticControllers; }
		
	private:
		static void Update();

		static void TransitionPressedKeys();
		static void TransitionPressedButtons();

		static void UpdateKeyState(KeyCode aKey, KeyState aNewState);
		static void UpdateButtonState(MouseButton aButton, KeyState aNewState);
		static void UpdateControllerButtonState(int aController, GamepadButton aButton, KeyState aNewState);

		static void ClearReleasedKeys();
		static void SetScrollValues(const CU::Vector2f& aScrollValues);

	private:
		inline static std::map<KeyCode, KeyData> staticKeyData;
		inline static std::map<MouseButton, ButtonData> staticMouseData;
		inline static std::map<int, Controller> staticControllers;

		inline static CU::Vector2f staticLastMousePos;
		inline static CU::Vector2f staticMouseDelta;
		inline static CU::Vector2f staticMouseScroll;

		friend class Application;
		friend class Window;
	};
}
