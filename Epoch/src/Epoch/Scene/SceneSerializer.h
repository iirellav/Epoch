#pragma once
#include <memory>
#include <filesystem>
#include "Scene.h"

namespace YAML
{
	class Emitter;
	class Node;
}

namespace Epoch
{
	class SceneSerializer
	{
	public:
		SceneSerializer(const std::shared_ptr<Scene>& aScene) : myScene(aScene) {}

		void Serialize(const std::filesystem::path& aFilepath);
		bool Deserialize(const std::filesystem::path& aFilepath);

		//void SerializeRuntime(const std::filesystem::path& aFilepath);
		//bool DeserializeRuntime(const std::filesystem::path& aFilepath);

		void SerializeEntities(YAML::Emitter& aOut);
		void DeserializeEntities(YAML::Node& aEntitiesNode);

	private:
		std::shared_ptr<Scene> myScene;
	};
}
