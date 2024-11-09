#include "InputHandler.h"

namespace CU
{
	void InputHandler::Init(HWND aWindowHandle)
	{
		if (myIsInitialized)
		{
			return;
		}

		myWindowHandle = aWindowHandle;

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC		((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE		((USHORT) 0x02)
#endif

		RAWINPUTDEVICE Rid[1]{};
		Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
		Rid[0].dwFlags = RIDEV_INPUTSINK;
		Rid[0].hwndTarget = myWindowHandle;

		RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));


		myIsInitialized = true;
	}

	void InputHandler::Reset()
	{
		myMousePosition = { 0, 0 };
		myTentativeMouseDelta = { 0, 0 };
		myMouseWheelDelta = { 0 ,0 };
		myInputState = { false };
	}

	bool InputHandler::UpdateEvents(UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_KEYDOWN:
			myInputState[wParam] = true;
			return true;

		case WM_KEYUP:
			myInputState[wParam] = false;
			return true;

		case WM_INPUT:
		{
			UINT dwSize = sizeof(RAWINPUT);
			static BYTE lpd[sizeof(RAWINPUT)];

			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpd, &dwSize, sizeof(RAWINPUTHEADER));

			RAWINPUT* raw = (RAWINPUT*)lpd;

			if (raw->header.dwType == RIM_TYPEMOUSE)
			{
				myTentativeMouseDelta.x = static_cast<LONG>(raw->data.mouse.lLastX);
				myTentativeMouseDelta.y = static_cast<LONG>(raw->data.mouse.lLastY);
			}
		}
		return true;

		case WM_MOUSEMOVE:
			myMousePosition.x = GET_X_LPARAM(lParam);
			myMousePosition.y = GET_Y_LPARAM(lParam);
			return true;

		case WM_MOUSEWHEEL:
			myMouseWheelDelta.y += GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
			return true;

		case WM_MOUSEHWHEEL:
			myMouseWheelDelta.x += GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
			return true;

		case WM_LBUTTONDOWN:
			myInputState[static_cast<int>(KeyCode::LEFTMBUTTON)] = true;
			return true;

		case WM_LBUTTONUP:
			myInputState[static_cast<int>(KeyCode::LEFTMBUTTON)] = false;
			return true;

		case WM_MBUTTONDOWN:
			myInputState[static_cast<int>(KeyCode::MIDDLEMBUTTON)] = true;
			return true;

		case WM_MBUTTONUP:
			myInputState[static_cast<int>(KeyCode::MIDDLEMBUTTON)] = false;
			return true;

		case WM_RBUTTONDOWN:
			myInputState[static_cast<int>(KeyCode::RIGHTMBUTTON)] = true;
			return true;

		case WM_RBUTTONUP:
			myInputState[static_cast<int>(KeyCode::RIGHTMBUTTON)] = false;
			return true;
		}

		return false;
	}

	void InputHandler::UpdateInput()
	{
		myCurrentMousePosition = myMousePosition;
		myMousePosition = { 0, 0 };

		myCurrentTentativeMouseDelta = myTentativeMouseDelta;
		myTentativeMouseDelta = { 0, 0 };

		myCurrentMouseWheelDelta = myMouseWheelDelta;
		myMouseWheelDelta = { 0 ,0 };

		myPreviusState = myCurrentState;
		myCurrentState = myInputState;
	}

	bool InputHandler::GetKeyPressed(const KeyCode aKeyCode)
	{
		int keyCode = static_cast<int>(aKeyCode);

		if (myCurrentState[keyCode] && !myPreviusState[keyCode])
		{
			return true;
		}

		return false;
	}

	bool InputHandler::GetKeyDown(const KeyCode aKeyCode)
	{
		int keyCode = static_cast<int>(aKeyCode);

		if (myCurrentState[keyCode])
		{
			return true;
		}

		return false;
	}

	bool InputHandler::GetKeyReleased(const KeyCode aKeyCode)
	{
		int keyCode = static_cast<int>(aKeyCode);

		if (!myCurrentState[keyCode] && myPreviusState[keyCode])
		{
			return true;
		}

		return false;
	}

	Vector2f InputHandler::GetMousePos()
	{
		return { static_cast<float>(myCurrentMousePosition.x), static_cast<float>(myCurrentMousePosition.y) };
	}

	Vector2f InputHandler::GetMouseDelta()
	{
		return { static_cast<float>(myCurrentTentativeMouseDelta.x), static_cast<float>(myCurrentTentativeMouseDelta.y) };
	}

	Vector2f InputHandler::GetMouseWheelDelta()
	{
		return { static_cast<float>(myCurrentMouseWheelDelta.x), static_cast<float>(myCurrentMouseWheelDelta.y) };
	}

	void InputHandler::ShowMouse()
	{
		CURSORINFO ci = { sizeof(CURSORINFO) };

		if (GetCursorInfo(&ci))
		{
			if (ci.flags == 0)
			{
				ShowCursor(true);
			}
		}
	}

	void InputHandler::HideMouse()
	{
		CURSORINFO ci = { sizeof(CURSORINFO) };

		if (GetCursorInfo(&ci))
		{
			if (ci.flags != 0)
			{
				ShowCursor(false);
			}
		}
	}

	void InputHandler::CaptureMouse()
	{
		RECT clipRect;

		GetClientRect(myWindowHandle, &clipRect);

		POINT upperLeft;
		upperLeft.x = clipRect.left;
		upperLeft.y = clipRect.top;

		POINT lowerRight;
		lowerRight.x = clipRect.right;
		lowerRight.y = clipRect.bottom;

		MapWindowPoints(myWindowHandle, nullptr, &upperLeft, 1);
		MapWindowPoints(myWindowHandle, nullptr, &lowerRight, 1);

		clipRect.left = upperLeft.x;
		clipRect.top = upperLeft.y;
		clipRect.right = lowerRight.x;
		clipRect.bottom = lowerRight.y;

		ClipCursor(&clipRect);
	}

	void InputHandler::CaptureMouse(const Vector2f& aMin, const Vector2f& aMax)
	{
		POINT min;
		min.x = (LONG)aMin.x;
		min.y = (LONG)aMin.y;

		POINT max;
		max.x = (LONG)aMax.x;
		max.y = (LONG)aMax.y;

		MapWindowPoints(myWindowHandle, nullptr, &min, 1);
		MapWindowPoints(myWindowHandle, nullptr, &max, 1);

		RECT clipRect;

		clipRect.left = min.x;
		clipRect.top = min.y;
		clipRect.right = max.x;
		clipRect.bottom = max.y;

		ClipCursor(&clipRect);
	}

	void InputHandler::ReleaseMouse()
	{
		ClipCursor(nullptr);
	}
}