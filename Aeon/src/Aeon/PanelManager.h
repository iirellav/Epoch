#pragma once
#include <memory>
#include <string>
#include <Epoch/Debug/Log.h>
#include <Epoch/Debug/Profiler.h>
#include <Epoch/Editor/EditorPanel.h>

namespace Epoch
{
	struct PanelData
	{
		std::string name = "";
		std::shared_ptr<EditorPanel> panel = nullptr;
		bool isOpen = false;
	};

	enum class PanelCategory { Edit, View, COUNT };

	class PanelManager
	{
	public:
		PanelManager() = default;
		~PanelManager() = default;

		void SetEntityDestroyedCallback(const std::function<void(Entity)>& callback) { myOnEntityDestroyedCallback = callback; }

		void OnImGuiRender();

		void OnEvent(Event& aEvent);

		void OnProjectChanged(const std::shared_ptr<Project>& aProject);
		void OnSceneChanged(const std::shared_ptr<Scene>& aScene);

		void Serialize();
		void Deserialize();

		template<typename T>
		std::shared_ptr<T> AddPanel(PanelCategory aCategory, const PanelData& aPanelData)
		{
			EPOCH_PROFILE_FUNC();

			static_assert(std::is_base_of<EditorPanel, T>::value, "PanelManager::AddPanel requires T to inherit from EditorPanel");

			auto& panelMap = myPanels[(size_t)aCategory];

			if (panelMap.find(aPanelData.name) != panelMap.end())
			{
				LOG_ERROR("A panel with the name '{}' has already been added.", aPanelData.name);
				return nullptr;
			}

			panelMap[aPanelData.name] = aPanelData;
			return std::dynamic_pointer_cast<T>(aPanelData.panel);
		}

		template<typename T>
		std::shared_ptr<T> AddPanel(PanelCategory aCategory, const std::string& aName, bool aIsOpenByDefault)
		{
			return AddPanel<T>(aCategory, PanelData{ aName, std::make_shared<T>(aName), aIsOpenByDefault });
		}

		PanelData* GetPanelData(const std::string& aName);

		template<typename T>
		std::shared_ptr<T> GetPanel(const std::string& aName)
		{
			static_assert(std::is_base_of<EditorPanel, T>::value, "PanelManager::AddPanel requires T to inherit from EditorPanel");

			for (const auto& panelMap : myPanels)
			{
				if (panelMap.find(aName) == panelMap.end())
				{
					continue;
				}

				return std::dynamic_pointer_cast<T>(panelMap.at(aName).panel);
			}

			LOG_ERROR("Couldn't find panel with name '{}'", aName);
			return nullptr;
		}

		std::unordered_map<std::string, PanelData>& GetPanels(PanelCategory aCategory) { return myPanels[(size_t)aCategory]; }

	private:
		std::array<std::unordered_map<std::string, PanelData>, (size_t)PanelCategory::COUNT> myPanels;
		
		std::function<void(Entity)> myOnEntityDestroyedCallback;
	};
}
