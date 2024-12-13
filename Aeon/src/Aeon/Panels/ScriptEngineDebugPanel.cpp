#include "ScriptEngineDebugPanel.h"
#include <Epoch/ImGui/ImGui.h>
#include <Epoch/Script/ScriptCache.h>
#include <Epoch/Script/ScriptEngine.h>
#include <Epoch/Editor/PanelIDs.h>

namespace Epoch
{
	static std::vector<AssemblyMetadata> staticLoadedAssembliesMetadata;

	void ScriptEngineDebugPanel::OnImGuiRender(bool& aIsOpen)
	{
		ImGui::Begin(SCRIPT_ENGINE_DEBUG_PANEL_ID, &aIsOpen);

		if (UI::PropertyGridHeader(fmt::format("Loaded Assemblies ({})", staticLoadedAssembliesMetadata.size()), false))
		{
			for (const auto& referencedAssembly : staticLoadedAssembliesMetadata)
			{
				ImGui::Text("%s (%d.%d.%d.%d)", referencedAssembly.name.c_str(), referencedAssembly.majorVersion, referencedAssembly.minorVersion, referencedAssembly.buildVersion, referencedAssembly.revisionVersion);
			}

			ImGui::TreePop();
		}

		if (UI::PropertyGridHeader("Cache", false))
		{
			const auto& cachedClasses = ScriptCache::GetCachedClasses();
			const auto& cachedFields = ScriptCache::GetCachedFields();
			const auto& cachedMethods = ScriptCache::GetCachedMethods();

			ImGui::Text("Total Classes: %d", cachedClasses.size());
			ImGui::Text("Total Fields: %d", cachedFields.size());
			ImGui::Text("Total Methods: %d", cachedMethods.size());

			for (const auto& [classID, managedClass] : cachedClasses)
			{
				const std::string label = fmt::format("{} ({})", managedClass.fullName, managedClass.id);

				if (label.find(std::string("System.")) != std::string::npos) continue;

				size_t unmanagedSize = sizeof(uint32_t) + managedClass.fullName.size() + (sizeof(uint32_t) * managedClass.fields.size()) + (sizeof(uint32_t) * managedClass.methods.size()) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(MonoClass*);
				if (UI::PropertyGridHeader(label.c_str(), false))
				{
					ImGui::Text("Unmanaged Size: %d", unmanagedSize);
					ImGui::Text("Managed Size: %d", managedClass.size);
					ImGui::TreePop();
				}
			}

			ImGui::TreePop();
		}

		ImGui::End();
	}

	void ScriptEngineDebugPanel::OnProjectChanged(const std::shared_ptr<Project>& aProject)
	{
		staticLoadedAssembliesMetadata.clear();

		const auto& epochCoreAssembly = ScriptEngine::GetCoreAssemblyInfo();
		const auto& appAssembly = ScriptEngine::GetAppAssemblyInfo();
		staticLoadedAssembliesMetadata.push_back(epochCoreAssembly->metadata);
		staticLoadedAssembliesMetadata.push_back(appAssembly->metadata);

		for (const auto& referencedAssemblyMetadata : epochCoreAssembly->referencedAssemblies)
		{
			staticLoadedAssembliesMetadata.push_back(referencedAssemblyMetadata);
		}

		for (const auto& referencedAssemblyMetadata : appAssembly->referencedAssemblies)
		{
			auto it = std::find_if(staticLoadedAssembliesMetadata.begin(), staticLoadedAssembliesMetadata.end(), [&referencedAssemblyMetadata](const AssemblyMetadata& aOther)
			{
				return referencedAssemblyMetadata == aOther;
			});

			if (it != staticLoadedAssembliesMetadata.end())
			{
				continue;
			}

			staticLoadedAssembliesMetadata.push_back(referencedAssemblyMetadata);
		}
	}
}
