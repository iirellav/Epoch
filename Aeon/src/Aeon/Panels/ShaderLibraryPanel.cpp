#include "ShaderLibraryPanel.h"
#include <Epoch/ImGui/ImGui.h>
#include <Epoch/Rendering/Renderer.h>
#include <Epoch/Rendering/Shader.h>
#include <Epoch/Editor/FontAwesome.h>

namespace Epoch
{
	void ShaderLibraryPanel::OnImGuiRender(bool& aIsOpen)
	{
		auto& shaders = Renderer::GetShaderLibrary()->GetShaders();

		ImGui::Begin(std::format("{}  Shader Library", EP_ICON_BOOK).c_str(), &aIsOpen);

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
