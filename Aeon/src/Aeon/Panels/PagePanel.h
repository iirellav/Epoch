#pragma once
#include <vector>
#include <functional>
#include <string>
#include <Epoch/Editor/EditorPanel.h>

namespace Epoch
{
	struct PanelPage
	{
		using PageRenderFunction = std::function<void()>;

		const char* name;
		PageRenderFunction renderFunction;
	};

	class PagePanel : public EditorPanel
	{
	public:
		PagePanel() = delete;
		PagePanel(const std::string& aName) : myName(aName) {}
		virtual ~PagePanel() override = default;

		void OnImGuiRender(bool& aIsOpen) override;

	protected:
		void DrawPageList();
		
	protected:
		int myCurrentPage = 0;
		std::vector<PanelPage> myPages;

	private:
		const std::string myName = "";
	};
}
