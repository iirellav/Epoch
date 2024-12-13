#pragma once
#include "Event.h"
#include "Epoch/Core/KeyCodes.h"

namespace Epoch
{
	class KeyEvent : public Event
	{
	public:
		inline KeyCode GetKeyCode() const { return myKeyCode; }

	protected:
		KeyEvent(KeyCode keycode) : myKeyCode(keycode) {}

		KeyCode myKeyCode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(KeyCode keycode) : KeyEvent(keycode) {}

		EVENT_CLASS_TYPE(KeyPressed)
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(KeyCode keycode) : KeyEvent(keycode) {}

		EVENT_CLASS_TYPE(KeyReleased)
	};

	class KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(KeyCode keycode) : KeyEvent(keycode) {}

		EVENT_CLASS_TYPE(KeyTyped)
	};
}