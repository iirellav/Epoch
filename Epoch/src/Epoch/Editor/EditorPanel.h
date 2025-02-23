#pragma once
#include <memory>
#include <Epoch/Project/Project.h>
#include <Epoch/Scene/Scene.h>
#include "Epoch/Core/Events/Event.h"

namespace Epoch
{
	class EditorPanel
	{
	public:
		EditorPanel() = delete;
		EditorPanel(const std::string& aName) : myName(aName) {}
		virtual ~EditorPanel() = default;

		virtual void OnImGuiRender(bool& aIsOpen) = 0;

		virtual void OnEvent(Event& aEvent) {}

		virtual void OnProjectChanged(const std::shared_ptr<Project>& aProject) { aProject; }
		virtual void OnSceneChanged(const std::shared_ptr<Scene>& aScene) { aScene; }

	protected:
		std::string myName;
	};
}
