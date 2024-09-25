#include "Mainmenu.h"

#include "DebugWindow.h"
#include "Explorer.h"
#include "ImDemo.h"
#include "Hierarchy/Hierarchy.h"
#include "Viewport/Viewport.h"

#include <Platform/Core/MainWindow.h>
#include <Renderer/GUI/CustomWidget.h>
#include <Renderer/GUI/GUI.h>
#include <Renderer/GUI/Icon.h>
#include <Wrapper/imgui.h>

namespace Cosmos::Editor
{
	Mainmenu::Mainmenu(Application* application)
		: Widget("Mainmenu"), mApplication(application)
	{
		mViewport = new Viewport();
		Renderer::GUI::GetRef().AddWidget(mViewport);

		mDebugWindow = new DebugWindow(mApplication);
		Renderer::GUI::GetRef().AddWidget(mDebugWindow);

		mExplorer = new Explorer();
		Renderer::GUI::GetRef().AddWidget(mExplorer);

		mImDemo = new ImDemo();
		Renderer::GUI::GetRef().AddWidget(mImDemo);

		mHierarchy = new Hierarchy(application);
		Renderer::GUI::GetRef().AddWidget(mHierarchy);
	}

	Mainmenu::~Mainmenu()
	{
	}

	void Mainmenu::OnUpdate()
	{	
		ImGui::BeginMainMenuBar();
		DisplayMenuItems();
		ImGui::EndMainMenuBar();
	}

	void Mainmenu::DisplayMenuItems()
	{
		if (ImGui::BeginMenu(ICON_FA_GLOBE " World"))
		{
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu(ICON_FA_EYE " View"))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 2.0f));

			ImGui::SeparatorText("Widgets");

			// imgui demo
			bool imdemo = mImDemo->IsOpened();
			if (ImGui::Checkbox("ImDemo", &imdemo)) {
				mImDemo->SetOpened(imdemo);
			}

			// info box
			bool info = mDebugWindow->IsOpened();
			if(ImGui::Checkbox("Info", &info)) {
				mDebugWindow->SetOpened(info);
			}

			// file explorer
			bool explorer = mExplorer->IsOpened();
			if (ImGui::Checkbox("Explorer", &explorer)) {
				mExplorer->SetOpened(explorer);
			}

			// scene hierarchy
			bool sceneHierarchy = mHierarchy->IsOpened();
			if (ImGui::Checkbox("Scene Hierarchy", &sceneHierarchy)) {
				mHierarchy->SetOpened(sceneHierarchy);
			}

			ImGui::PopStyleVar();
			ImGui::EndMenu();
		}
		
		if (ImGui::BeginMenu(ICON_FA_WRENCH " Settings"))
		{
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu(ICON_FA_BUG " Debug"))
		{
			ImGui::EndMenu();
		}
	}
}