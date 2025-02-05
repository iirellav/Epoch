#pragma once
#include <memory>
#include <filesystem>
#include "Scene.h"
#include "Epoch/Serialization/FileStream.h"
#include "Epoch/Assets/AssetSerializer/AssetSerializer.h"

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

		bool SerializeToAssetPack(FileStreamWriter& aStream, AssetSerializationInfo& outInfo);
		bool DeserializeFromAssetPack(FileStreamReader& aStream, const AssetPackFile::SceneInfo& aSceneInfo);

	private:
		void SerializeToYAML(YAML::Emitter& out);
		bool DeserializeFromYAML(const std::string& aYamlString);

	private:
		std::shared_ptr<Scene> myScene;
	};
}
