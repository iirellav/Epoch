#pragma once
#pragma comment(lib, "XInput.lib")
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>
#include "../Math/Vector/Vector2.hpp"

enum class XKeyCode;

namespace CU
{
	class XInputHandler
	{
	public:
		XInputHandler();
		~XInputHandler() = default;

		void Unload();
		bool IsConnected();
		const XINPUT_GAMEPAD& GetState();

		void Update();

		bool GetKeyDown(XKeyCode aButton);
		bool GetKeyUp(XKeyCode aButton);
		bool GetKey(XKeyCode aButton);

		bool GetLeftTriggerDown();
		bool GetLeftTriggerUp();
		bool GetLeftTrigger();
		float GetLeftTriggerValue();

		bool GetRightTriggerDown();
		bool GetRightTriggerUp();
		bool GetRightTrigger();
		float GetRightTriggerValue();

		Vector2<float> GetLeftThumbstick();
		float GetLeftThumbstickX();
		float GetLeftThumbstickY();

		Vector2<float> GetRightThumbstick();
		float GetRightThumbstickX();
		float GetRightThumbstickY();

		int GetLeftVibration();
		int GetRightVibration();

		void ToggleVibration(bool aBool);
		bool GetVibrationEnabled();

		void Vibrate(int aLeftValue, int aRightValue);
		void Vibrate(int aValue);

		void VibrateForSeconds(int aLeftValue, int aRightValue, float aTimespan);
		void VibrateForSeconds(int aValue, float aTimespan);

	private:
		bool myVibrationEnabled;
		bool myVibrationTimerActive;

		XINPUT_STATE myPreviousState;
		XINPUT_STATE myCurrentState;
		XINPUT_VIBRATION myVibration;

		float myThumbstickDeadzone;
		float myLeftThumbstickX;
		float myLeftThumbstickY;
		float myRightThumbstickX;
		float myRightThumbstickY;

		float myTriggerDeadzone;

		float myLeftTrigger;
		float myPreviousLeftTrigger;

		float myRightTrigger;
		float myPreviousRightTrigger;

		int myLeftVibration;
		int myRightVibration;

		float myVibrationTimer;

		const int myVibrationMaxValue = 65535;
		const float myTriggerValueMultiplier = 0.0039215f;
		const float myThumbstickValueMultiplier = 0.0000305f;

		void UpdateButtons();
		void UpdateTrigger();
		void UpdateThumbstick();
		void UpdateVibration();
	};
}

enum class XKeyCode : int
{
	Up = 0x0001,
	Down = 0x0002,
	Left = 0x0004,
	Right = 0x0008,
	Start = 0x0010,
	Back = 0x0020,
	LeftThumbstick = 0x0040,
	RightThumbstick = 0x0080,
	LeftShoulder = 0x0100,
	RightShoulder = 0x0200,
	A = 0x1000,
	B = 0x2000,
	X = 0x4000,
	Y = 0x8000
};
