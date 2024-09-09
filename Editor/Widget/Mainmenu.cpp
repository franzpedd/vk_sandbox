#include "Mainmenu.h"

#include "DebugWindow.h"
#include "Explorer.h"
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
		mDebugWindow = new DebugWindow(mApplication);
		Renderer::GUI::GetRef().AddWidget(mDebugWindow);

		mExplorer = new Explorer();
		Renderer::GUI::GetRef().AddWidget(mExplorer);

	}

	Mainmenu::~Mainmenu()
	{
	}

	void Mainmenu::OnUpdate()
	{
		// set the window into the correct position and it's flags
		//int drawableWidth, drawableHeight;
		//Platform::MainWindow::GetRef().GetFrameBufferSize(&drawableWidth, &drawableHeight);
		//
		//int relativeX, relativeY;
		//Platform::MainWindow::GetRef().GetRelativePosition(&relativeX, &relativeY);
		//
		//ImGui::SetNextWindowSize(ImVec2((float)drawableWidth, 50.0f));
		//ImGui::SetNextWindowPos(ImVec2((float)relativeX, (float)relativeY));
		//ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar 
		//	| ImGuiWindowFlags_NoResize
		//	| ImGuiWindowFlags_NoMove
		//	| ImGuiWindowFlags_NoScrollbar
		//	| ImGuiWindowFlags_NoCollapse ; //ImGuiWindowFlags_;
		
		ImGui::BeginMainMenuBar();
		DisplayMenuItems();
		ImGui::EndMainMenuBar();
	}

	void Mainmenu::DisplayMenuItems()
	{
		if (ImGui::BeginMenu(ICON_LC_MAP " World"))
		{
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu(ICON_LC_VIEW " View"))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 2.0f));

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

			ImGui::PopStyleVar();
			ImGui::EndMenu();
		}
		
		if (ImGui::BeginMenu(ICON_FA_COG " Settings"))
		{
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu(ICON_LC_SEARCH " Debug"))
		{
			ImGui::EndMenu();
		}
	}
}