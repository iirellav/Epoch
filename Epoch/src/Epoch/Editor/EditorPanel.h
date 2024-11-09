#pragma once
#include <memory>
#include <Epoch/Project/Project.h>
#include <Epoch/Scene/Scene.h>

namespace Epoch
{
	class EditorPanel
	{
	public:
		virtual ~EditorPanel() = default;

		virtual void OnImGuiRender(bool& aIsOpen) = 0;
		virtual void OnProjectChanged(const std::shared_ptr<Project>& aProject) { aProject; }
		virtual void OnSceneChanged(const std::shared_ptr<Scene>& aScene) { aScene; }
	};
}
