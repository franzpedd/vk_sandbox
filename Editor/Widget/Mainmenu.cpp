#include "Mainmenu.h"

#include "DebugWindow.h"
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
			bool s_info = mDebugWindow->IsOpened();
			if(ImGui::Checkbox("Info", &s_info)) {
				mDebugWindow->SetOpened(s_info);
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