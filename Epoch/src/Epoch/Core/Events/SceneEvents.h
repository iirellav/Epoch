#pragma once
#include "Event.h"
#include <memory>
#include "Epoch/Scene/Scene.h"

namespace Epoch
{
	class SceneEvent : public Event
	{
	public:
		const std::shared_ptr<Scene>& GetScene() const { return myScene; }
		std::shared_ptr<Scene> GetScene() { return myScene; }

	protected:
		SceneEvent(const std::shared_ptr<Scene>& scene) : myScene(scene) {}

		std::shared_ptr<Scene> myScene;
	};

	class ScenePreStartEvent : public SceneEvent
	{
	public:
		ScenePreStartEvent(const std::shared_ptr<Scene>& aScene) : SceneEvent(aScene) {}

		EVENT_CLASS_TYPE(ScenePreStart)
	};

	class ScenePostStartEvent : public SceneEvent
	{
	public:
		ScenePostStartEvent(const std::shared_ptr<Scene>& aScene) : SceneEvent(aScene) {}

		EVENT_CLASS_TYPE(ScenePostStart)
	};

	class ScenePreStopEvent : public SceneEvent
	{
	public:
		ScenePreStopEvent(const std::shared_ptr<Scene>& aScene) : SceneEvent(aScene) {}

		EVENT_CLASS_TYPE(ScenePreStop)
	};

	class ScenePostStopEvent : public SceneEvent
	{
	public:
		ScenePostStopEvent(const std::shared_ptr<Scene>& aScene) : SceneEvent(aScene) {}

		EVENT_CLASS_TYPE(ScenePostStop)
	};
}
