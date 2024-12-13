#pragma once
#include <functional>

namespace Epoch
{
	enum class EventType
	{
		None,
		WindowClose, WindowMinimize, WindowResize,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
		ScenePreStart, ScenePostStart, ScenePreStop, ScenePostStop,
		EditorExitPlayMode, EditorFileDropped
	};

#define EVENT_CLASS_TYPE(aType) static EventType GetStaticType() { return EventType::aType; }\
								EventType GetEventType() const override { return GetStaticType(); }

	class Event
	{
	public:
		Event() = default;
		virtual ~Event() = default;

		bool IsHandled() const { return myIsHandled; }

		virtual EventType GetEventType() const = 0;

	private:
		bool myIsHandled = false;

		friend class EventDispatcher;
	};

	class EventDispatcher
	{
		template<typename T>
		using EventFn = std::function<bool(T&)>;
	public:
		EventDispatcher(Event& aEvent) : myEvent(aEvent) {}

		template<typename T>
		bool Dispatch(EventFn<T> aFunc)
		{
			if (myEvent.GetEventType() != T::GetStaticType() || myEvent.myIsHandled)
			{
				return false;
			}
			
			myEvent.myIsHandled = aFunc(*(T*)&myEvent);
			return true;
		}
	private:
		Event& myEvent;
	};
}
