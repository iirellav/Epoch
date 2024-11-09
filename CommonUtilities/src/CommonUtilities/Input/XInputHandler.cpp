#include "XInputHandler.h"
#include "../Math/CommonMath.hpp"
#include "../Timer.h"

namespace CU
{
	XInputHandler::XInputHandler()
	{
		myVibrationEnabled = true;
		myVibrationTimerActive = false;

		ZeroMemory(&myCurrentState, sizeof(XINPUT_STATE));
		ZeroMemory(&myVibration, sizeof(XINPUT_VIBRATION));
		
		myPreviousState = {};
		myCurrentState = {};

		myThumbstickDeadzone = 0.1f;
		myLeftThumbstickX = 0;
		myLeftThumbstickY = 0;
		myRightThumbstickX = 0;
		myRightThumbstickY = 0;

		myTriggerDeadzone = 0.05f;

		myLeftTrigger = 0;
		myRightTrigger = 0;

		myLeftVibration = 0;
		myRightVibration = 0;

		myVibrationTimer = 0;
	}

	void XInputHandler::Unload()
	{
		ZeroMemory(&myCurrentState, sizeof(XINPUT_STATE));
		ZeroMemory(&myVibration, sizeof(XINPUT_VIBRATION));

		XInputSetState(0, &myVibration);
	}

	bool XInputHandler::IsConnected()
	{
		for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
		{
			XINPUT_STATE state;
			ZeroMemory(&state, sizeof(XINPUT_STATE));

			if (XInputGetState(i, &state) == ERROR_SUCCESS)
			{
				return true;
			}
		}

		return false;
	}

	const XINPUT_GAMEPAD& XInputHandler::GetState()
	{
		return myCurrentState.Gamepad;
	}

	void XInputHandler::Update()
	{
		UpdateButtons();

		UpdateTrigger();

		UpdateThumbstick();

		UpdateVibration();
	}

	bool XInputHandler::GetKeyDown(XKeyCode aButton)
	{
		DWORD keyCode = static_cast<DWORD>(aButton);

		if ((myCurrentState.Gamepad.wButtons & keyCode) && !(myPreviousState.Gamepad.wButtons & keyCode))
		{
			return true;
		}

		return false;
	}

	bool XInputHandler::GetKeyUp(XKeyCode aButton)
	{
		DWORD keyCode = static_cast<DWORD>(aButton);

		if (!(myCurrentState.Gamepad.wButtons & keyCode) && (myPreviousState.Gamepad.wButtons & keyCode))
		{
			return true;
		}

		return false;
	}

	bool XInputHandler::GetKey(XKeyCode aButton)
	{
		DWORD keyCode = static_cast<DWORD>(aButton);

		if ((myCurrentState.Gamepad.wButtons & keyCode))
		{
			return true;
		}

		return false;
	}

	bool XInputHandler::GetLeftTriggerDown()
	{
		if ((myLeftTrigger > 0) && !(myPreviousLeftTrigger > 0))
		{
			return true;
		}

		return false;
	}

	bool XInputHandler::GetLeftTriggerUp()
	{
		if (!(myLeftTrigger > 0) && (myPreviousLeftTrigger > 0))
		{
			return true;
		}

		return false;
	}

	bool XInputHandler::GetLeftTrigger()
	{
		if ((myLeftTrigger > 0))
		{
			return true;
		}

		return false;
	}

	float XInputHandler::GetLeftTriggerValue()
	{
		return myLeftTrigger;
	}

	bool XInputHandler::GetRightTriggerDown()
	{
		if ((myRightTrigger > 0) && !(myPreviousRightTrigger > 0))
		{
			return true;
		}

		return false;
	}

	bool XInputHandler::GetRightTriggerUp()
	{
		if (!(myRightTrigger > 0) && (myPreviousRightTrigger > 0))
		{
			return true;
		}

		return false;
	}

	bool XInputHandler::GetRightTrigger()
	{
		if ((myRightTrigger > 0))
		{
			return true;
		}

		return false;
	}

	float XInputHandler::GetRightTriggerValue()
	{
		return myRightTrigger;
	}

	Vector2<float> XInputHandler::GetLeftThumbstick()
	{
		return { myLeftThumbstickX, myLeftThumbstickY };
	}

	float XInputHandler::GetLeftThumbstickX()
	{
		return myLeftThumbstickX;
	}

	float XInputHandler::GetLeftThumbstickY()
	{
		return myLeftThumbstickY;
	}

	Vector2<float> XInputHandler::GetRightThumbstick()
	{
		return {myRightThumbstickX, myRightThumbstickY};
	}

	float XInputHandler::GetRightThumbstickX()
	{
		return myRightThumbstickX;
	}

	float XInputHandler::GetRightThumbstickY()
	{
		return myRightThumbstickY;
	}

	int XInputHandler::GetLeftVibration()
	{
		return myLeftVibration;
	}

	int XInputHandler::GetRightVibration()
	{
		return myRightVibration;
	}

	void XInputHandler::ToggleVibration(bool aBool)
	{
		myVibrationEnabled = aBool;
	}

	bool XInputHandler::GetVibrationEnabled()
	{
		return myVibrationEnabled;
	}

	void XInputHandler::Vibrate(int aLeftValue, int aRightValue)
	{
		myLeftVibration = Math::Clamp(aLeftValue, 0, myVibrationMaxValue);

		myRightVibration = Math::Clamp(aRightValue, 0, myVibrationMaxValue);
	}

	void XInputHandler::Vibrate(int aValue)
	{
		myLeftVibration = Math::Clamp(aValue, 0, myVibrationMaxValue);

		myRightVibration = Math::Clamp(aValue, 0, myVibrationMaxValue);
	}

	void XInputHandler::VibrateForSeconds(int aLeftValue, int aRightValue, float aTimespan)
	{
		myVibrationTimerActive = true;
		myVibrationTimer = aTimespan;

		myLeftVibration = Math::Clamp(aLeftValue, 0, myVibrationMaxValue);

		myRightVibration = Math::Clamp(aRightValue, 0, myVibrationMaxValue);
	}

	void XInputHandler::VibrateForSeconds(int aValue, float aTimespan)
	{
		myVibrationTimerActive = true;
		myVibrationTimer = aTimespan;

		myLeftVibration = Math::Clamp(aValue, 0, myVibrationMaxValue);

		myRightVibration = Math::Clamp(aValue, 0, myVibrationMaxValue);
	}
	
	void XInputHandler::UpdateButtons()
	{
		myPreviousState.Gamepad.wButtons = myCurrentState.Gamepad.wButtons;

		ZeroMemory(&myCurrentState, sizeof(XINPUT_STATE));

		XInputGetState(0, &myCurrentState);
	}

	void XInputHandler::UpdateTrigger()
	{
		myPreviousLeftTrigger = myLeftTrigger;
		myPreviousRightTrigger = myRightTrigger;

		myLeftTrigger = static_cast<float>(myCurrentState.Gamepad.bLeftTrigger) * myTriggerValueMultiplier;
		if (myLeftTrigger < myTriggerDeadzone)
		{
			myLeftTrigger = 0;
		}

		myRightTrigger = static_cast<float>(myCurrentState.Gamepad.bRightTrigger) * myTriggerValueMultiplier;
		if (myRightTrigger < myTriggerDeadzone)
		{
			myRightTrigger = 0;
		}
	}

	void XInputHandler::UpdateThumbstick()
	{
		myLeftThumbstickX = myCurrentState.Gamepad.sThumbLX * myThumbstickValueMultiplier;
		if (CU::Math::Abs(myLeftThumbstickX) < myThumbstickDeadzone)
		{
			myLeftThumbstickX = 0;
		}

		myLeftThumbstickY = myCurrentState.Gamepad.sThumbLY * myThumbstickValueMultiplier;
		if (CU::Math::Abs(myLeftThumbstickY) < myThumbstickDeadzone)
		{
			myLeftThumbstickY = 0;
		}


		myRightThumbstickX = myCurrentState.Gamepad.sThumbRX * myThumbstickValueMultiplier;
		if (CU::Math::Abs(myRightThumbstickX) < myThumbstickDeadzone)
		{
			myRightThumbstickX = 0;
		}

		myRightThumbstickY = myCurrentState.Gamepad.sThumbRY * myThumbstickValueMultiplier;
		if (CU::Math::Abs(myRightThumbstickY) < myThumbstickDeadzone)
		{
			myRightThumbstickY = 0;
		}
	}

	void XInputHandler::UpdateVibration()
	{
		ZeroMemory(&myVibration, sizeof(XINPUT_VIBRATION));

		if (myVibrationEnabled)
		{
			myVibration.wLeftMotorSpeed = static_cast<WORD>(myLeftVibration);
			myVibration.wRightMotorSpeed = static_cast<WORD>(myRightVibration);

			if (myVibrationTimerActive)
			{
				if (myVibrationTimer <= 0)
				{
					myVibrationTimerActive = false;

					myLeftVibration = 0;
					myRightVibration = 0;
				}
				else
				{
					myVibrationTimer -= Timer::GetDeltaTime();
				}
			}
		}

		XInputSetState(0, &myVibration);
	}
}
