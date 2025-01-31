#include "epch.h"
#include "ProjectSerializer.h"
#include "Epoch/Physics/PhysicsSystem.h"
#pragma warning(push, 0)
#include <yaml-cpp/yaml.h>
#pragma warning(pop)

namespace Epoch
{
	static void SerializePhysicsLayers(YAML::Emitter& aOut)
	{
		aOut << YAML::Key << "Layers";
		aOut << YAML::Value << YAML::BeginSeq;
		for (const auto& layer : PhysicsLayerManager::GetLayers())
		{
			if (layer.reserved || layer.name == "")
			{
				continue;
			}

			aOut << YAML::BeginMap;
			aOut << YAML::Key << "Name" << YAML::Value << layer.name;
			aOut << YAML::Key << "ID" << YAML::Value << layer.layerID;
			aOut << YAML::Key << "CollidesWith" << YAML::Value;
			aOut << YAML::BeginSeq;
			for (const auto& collidingLayer : PhysicsLayerManager::GetLayerCollisions(layer.layerID))
			{
				if (collidingLayer.name == "")
				{
					continue;
				}

				aOut << YAML::BeginMap;
				aOut << YAML::Key << "Name" << YAML::Value << collidingLayer.name;
				aOut << YAML::EndMap;
			}
			aOut << YAML::EndSeq;
			aOut << YAML::EndMap;
		}
		aOut << YAML::EndSeq;
	}

	static void DeserializePhysicsLayers(YAML::Node& aNode)
	{
		auto physicsLayers = aNode["Layers"];
		if (physicsLayers)
		{
			for (auto layer : physicsLayers)
			{
				PhysicsLayerManager::UpdateLayerName(layer["ID"].as<uint32_t>(1), layer["Name"].as<std::string>(""));
			}

			for (auto layer : physicsLayers)
			{
				auto collidesWith = layer["CollidesWith"];
				if (collidesWith)
				{
					for (auto collisionLayer : collidesWith)
					{
						const auto& firstLayer = PhysicsLayerManager::GetLayer(layer["Name"].as<std::string>(""));
						const auto& otherLayer = PhysicsLayerManager::GetLayer(collisionLayer["Name"].as<std::string>(""));
						PhysicsLayerManager::SetLayerCollision(firstLayer.layerID, otherLayer.layerID, true);
					}
				}
			}
		}
	}

	void ProjectSerializer::Serialize(const std::filesystem::path& aFilepath)
	{
		EPOCH_PROFILE_FUNC();

		auto& config = myProject->myConfig;
		LOG_DEBUG("Serializing project '{}'", config.name);
		
		//Project config
		{
			auto& physicsSettings = PhysicsSystem::GetSettings();

			YAML::Emitter out;
			out << YAML::BeginMap;
			out << YAML::Key << "Project" << YAML::Value;
			{
				out << YAML::BeginMap;
				out << YAML::Key << "Name" << YAML::Value << config.name;
				out << YAML::Key << "ProductName" << YAML::Value << config.productName;
				out << YAML::Key << "CompanyName" << YAML::Value << config.companyName;
				out << YAML::Key << "Version" << YAML::Value << config.version;
				out << YAML::Key << "IconPath" << YAML::Value << config.iconPath.string();

				out << YAML::Key << "StartScene" << YAML::Value << config.startScene;

				out << YAML::Key << "AutosaveDirectory" << YAML::Value << config.autosaveDirectory.string();

				out << YAML::Key << "AssetDirectory" << YAML::Value << config.assetDirectory.string();
				out << YAML::Key << "AssetRegistryPath" << YAML::Value << config.assetRegistryPath.string();

				out << YAML::Key << "DefaultScriptNamespace" << YAML::Value << config.defaultScriptNamespace;

				out << YAML::Key << "FixedTimestep" << YAML::Value << physicsSettings.fixedTimestep;
				out << YAML::Key << "Gravity" << YAML::Value << physicsSettings.gravity;

				SerializePhysicsLayers(out);

				out << YAML::EndMap;
			}
			out << YAML::EndMap;

			std::ofstream fileOut(aFilepath);
			fileOut << out.c_str();
			fileOut.close();
		}

		//User config
		{
			const std::filesystem::path userConfigPath = aFilepath.parent_path() / "Config/UserConfig.config";
			if (!FileSystem::Exists(userConfigPath.parent_path()))
			{
				FileSystem::CreateDirectory(userConfigPath.parent_path());
			}

			auto& userConfig = myProject->myUserConfig;

			YAML::Emitter out;
			out << YAML::BeginMap;
			out << YAML::Key << "Config" << YAML::Value;
			{
				out << YAML::BeginMap;
				
				out << YAML::Key << "StartScene" << YAML::Value << userConfig.startScene.string();
				out << YAML::Key << "EditorCameraPosition" << YAML::Value << userConfig.editorCameraPosition;
				out << YAML::Key << "EditorCameraRotation" << YAML::Value << userConfig.editorCameraRotation;

				out << YAML::Key << "RecentScenes";
				out << YAML::Value << YAML::BeginSeq;
				for (const auto& [lastOpened, sceneConfig] : userConfig.recentScenes)
				{
					out << YAML::BeginMap;
					out << YAML::Key << "Name" << YAML::Value << sceneConfig.name;
					out << YAML::Key << "ScenePath" << YAML::Value << sceneConfig.filePath;
					out << YAML::Key << "LastOpened" << YAML::Value << sceneConfig.lastOpened;
					out << YAML::EndMap;
				}
				out << YAML::EndSeq;

				out << YAML::EndMap;
			}
			out << YAML::EndMap;

			std::ofstream fileOut(userConfigPath);
			fileOut << out.c_str();
			fileOut.close();
		}
	}

	bool ProjectSerializer::Deserialize(const std::filesystem::path& aFilepath)
	{
		EPOCH_PROFILE_FUNC();

		//Project config
		{
			YAML::Node data;
			try
			{
				data = YAML::LoadFile(aFilepath.string());
			}
			catch (YAML::ParserException e)
			{
				LOG_ERROR("Failed to load .epoch file '{}'\n     {}", aFilepath.string(), e.what());
				return false;
			}

			if (!data["Project"])
			{
				return false;
			}


			YAML::Node rootNode = data["Project"];

			auto& config = myProject->myConfig;
			auto& physicsSettings = PhysicsSystem::GetSettings();

			config.name = rootNode["Name"].as<std::string>();
			LOG_DEBUG("Deserializing project '{}'", myProject->myConfig.name);
			config.productName = rootNode["ProductName"].as<std::string>(config.name);

			const std::filesystem::path& projectPath = aFilepath;
			config.projectFileName = projectPath.filename().string();
			config.projectDirectory = projectPath.parent_path().string();

			config.companyName = rootNode["CompanyName"].as<std::string>("");
			config.version = rootNode["Version"].as<std::string>(config.version);
			config.iconPath = (std::filesystem::path)rootNode["IconPath"].as<std::string>("");

			config.startScene = rootNode["StartScene"].as<UUID>(UUID(0));

			config.autosaveDirectory = rootNode["AutosaveDirectory"].as<std::string>("Autosaves");
			if (!std::filesystem::exists(config.projectDirectory / config.autosaveDirectory))
			{
				LOG_WARNING("Autosave directory does not exist!");
				LOG_INFO("Autosave directory created");
				std::filesystem::create_directory(config.projectDirectory / config.autosaveDirectory);
			}

			config.assetDirectory = rootNode["AssetDirectory"].as<std::string>("Assets");
			if (!std::filesystem::exists(config.projectDirectory / config.assetDirectory))
			{
				LOG_WARNING("Asset directory does not exist!");
				LOG_INFO("Asset directory created");
				std::filesystem::create_directory(config.projectDirectory / config.assetDirectory);
			}

			config.assetRegistryPath = rootNode["AssetRegistryPath"].as<std::string>(config.assetRegistryPath.string());

			config.defaultScriptNamespace = rootNode["DefaultScriptNamespace"].as<std::string>(config.projectFileName.stem().string());
			config.defaultScriptNamespace = CU::RemoveWhitespaces(config.defaultScriptNamespace);

			physicsSettings.fixedTimestep = rootNode["FixedTimestep"].as<float>(1.0f / 60.0f);
			physicsSettings.gravity = rootNode["Gravity"].as<CU::Vector3f>(CU::Vector3f(0.0f, -982.0f, 0.0f));

			DeserializePhysicsLayers(rootNode);
		}

		//User config
		{
			const std::filesystem::path userConfigPath = aFilepath.parent_path() / "Config/UserConfig.config";

			if (!FileSystem::Exists(userConfigPath))
			{
				return true;
			}

			YAML::Node data;
			try
			{
				data = YAML::LoadFile(userConfigPath.string());
			}
			catch (YAML::ParserException e)
			{
				LOG_ERROR("Failed to load .config file '{}'\n     {}", userConfigPath.string(), e.what());
				return false;
			}

			if (!data["Config"])
			{
				return false;
			}


			YAML::Node rootNode = data["Config"];

			auto& userConfig = myProject->myUserConfig;

			userConfig.startScene = rootNode["StartScene"].as<std::string>("");
			userConfig.editorCameraPosition = rootNode["EditorCameraPosition"].as<CU::Vector3f>(CU::Vector3f::Zero);
			userConfig.editorCameraRotation = rootNode["EditorCameraRotation"].as<CU::Vector3f>(CU::Vector3f::Zero);

			for (auto recentProject : rootNode["RecentScenes"])
			{
				RecentScene entry;
				entry.name = recentProject["Name"].as<std::string>();
				entry.filePath = recentProject["ScenePath"].as<std::string>();
				entry.lastOpened = recentProject["LastOpened"].as<time_t>(time(NULL));
				if (!FileSystem::Exists(entry.filePath)) continue;
				userConfig.recentScenes[entry.lastOpened] = entry;
			}
		}

		return true;
	}

	void ProjectSerializer::SerializeRuntime(const std::filesystem::path& aFilepath)
	{
		EPOCH_PROFILE_FUNC();

		auto& config = myProject->myConfig;
		auto& physicsSettings = PhysicsSystem::GetSettings();

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Project" << YAML::Value;
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Name" << YAML::Value << config.name;
			out << YAML::Key << "ProductName" << YAML::Value << config.productName;
			out << YAML::Key << "CompanyName" << YAML::Value << config.companyName;
			out << YAML::Key << "Version" << YAML::Value << config.version;

			out << YAML::Key << "StartScene" << YAML::Value << config.startScene;
			
			out << YAML::Key << "FixedTimestep" << YAML::Value << physicsSettings.fixedTimestep;
			out << YAML::Key << "Gravity" << YAML::Value << physicsSettings.gravity;

			SerializePhysicsLayers(out);

			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		LOG_DEBUG("Serializing runtime project '{}'", config.name);

		FileStreamWriter stream(aFilepath);
		stream.WriteString(std::string(out.c_str()));
	}

	bool ProjectSerializer::DeserializeRuntime(const std::filesystem::path& aFilepath)
	{
		EPOCH_PROFILE_FUNC();

		FileStreamReader stream(aFilepath);
		std::string yamlString;
		stream.ReadString(yamlString);

		YAML::Node data = YAML::Load(yamlString);

		if (!data["Project"])
		{
			return false;
		}

		YAML::Node rootNode = data["Project"];

		auto& config = myProject->myConfig;
		auto& physicsSettings = PhysicsSystem::GetSettings();

		config.name = rootNode["Name"].as<std::string>();
		LOG_DEBUG("Deserializing project '{}'", myProject->myConfig.name);
		config.productName = rootNode["ProductName"].as<std::string>(config.name);

		const std::filesystem::path& projectPath = aFilepath;
		config.projectFileName = projectPath.filename().string();
		config.projectDirectory = projectPath.parent_path().string();
		
		config.companyName = rootNode["CompanyName"].as<std::string>("");
		config.version = rootNode["Version"].as<std::string>(config.version);

		config.startScene = rootNode["StartScene"].as<UUID>(UUID(0));
		
		physicsSettings.fixedTimestep = rootNode["FixedTimestep"].as<float>(1.0f/60.0f);
		physicsSettings.gravity = rootNode["Gravity"].as<CU::Vector3f>(CU::Vector3f(0.0f, -982.0f, 0.0f));

		DeserializePhysicsLayers(rootNode);

		return true;
	}
}
