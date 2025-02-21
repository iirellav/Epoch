#include "ShaderLibraryPanel.h"
#include <Epoch/ImGui/ImGui.h>
#include <Epoch/Rendering/Renderer.h>
#include <Epoch/Rendering/Shader.h>

namespace Epoch
{
	ShaderLibraryPanel::ShaderLibraryPanel(const std::string& aName) : EditorPanel(aName)
	{
	}

	void ShaderLibraryPanel::OnImGuiRender(bool& aIsOpen)
	{
		auto& shaders = Renderer::GetShaderLibrary()->GetShaders();

		bool open = ImGui::Begin(myName.c_str(), &aIsOpen);
		
		if (!open)
		{
			ImGui::End();
			return;
		}

		static std::string searchString;
		ImGui::InputTextWithHint("##Search", "Search...", &searchString, ImGuiInputTextFlags_AutoSelectAll);
		
		ImGui::SameLine();

		if (ImGui::Button("Reload All"))
		{
			for (auto& [name, shader] : shaders)
			{
				Renderer::GetShaderLibrary()->Reload(name);
			}
		}

		ImGui::SameLine();

		ImGui::TextDisabled("%i", shaders.size());

		ImGui::Separator();
		ImGui::Spacing();
		
		ImGui::BeginChild("ShaderLibView");

		for (auto& [name, shader] : shaders)
		{
			if (!UI::IsMatchingSearch(name, searchString)) continue;

			ImGui::Columns(2);

			ImGui::Text(name.c_str());

			ImGui::NextColumn();

			std::string buttonName = std::format("Open##{0}", name);
			if (ImGui::Button(buttonName.c_str()))
			{
				FileSystem::OpenExternally(shader->GetFilePath());
			}

			ImGui::SameLine();

			buttonName = std::format("Reload##{0}", name);
			if (ImGui::Button(buttonName.c_str()))
			{
				Renderer::GetShaderLibrary()->Reload(name);
			}

			ImGui::Columns(1);
			ImGui::Spacing();
			ImGui::Spacing();
		}

		ImGui::EndChild();

		ImGui::End();
	}
}
