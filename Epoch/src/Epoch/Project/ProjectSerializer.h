#pragma once
#include <memory>
#include "Project.h"

namespace Epoch
{
	class ProjectSerializer
	{
	public:
		ProjectSerializer(std::shared_ptr<Project> aProject) : myProject(aProject) {}

		void Serialize(const std::filesystem::path& aFilepath);
		bool Deserialize(const std::filesystem::path& aFilepath);
		
		void SerializeRuntime(const std::filesystem::path& aFilepath);
		bool DeserializeRuntime(const std::filesystem::path& aFilepath);

	private:
		std::shared_ptr<Project> myProject;
	};
}
