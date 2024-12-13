#pragma once
#include "Event.h"
#include <string>

namespace Epoch
{
	class EditorExitPlayModeEvent : public Event
	{
	public:
		EditorExitPlayModeEvent() = default;

		EVENT_CLASS_TYPE(EditorExitPlayMode)
	};

	class EditorFileDroppedEvent : public Event
	{
	public:
		EditorFileDroppedEvent(std::vector<std::string> aPaths) : myPaths(aPaths) {}

		const std::vector<std::string>& GetPaths() const { return myPaths; }

		EVENT_CLASS_TYPE(EditorFileDropped)

	private:
		std::vector<std::string> myPaths;
	};
}
