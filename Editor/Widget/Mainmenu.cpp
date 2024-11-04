#include "Mainmenu.h"

#include "DebugWindow.h"
#include "Console.h"
#include "Explorer.h"
#include "ImDemo.h"
#include "Core/Application.h"
#include "PrefabHierarchy.h"
#include "Viewport/Viewport.h"

#include <Common/Debug/Logger.h>
#include <Common/File/Filesystem.h>
#include <Engine/Core/Scene.h>
#include <Platform/Core/MainWindow.h>
#include <Renderer/GUI/CustomWidget.h>
#include <Renderer/GUI/GUI.h>
#include <Renderer/GUI/Icon.h>
#include <Wrapper/imgui.h>

#include <filesystem>

namespace Cosmos::Editor
{
	Mainmenu::Mainmenu(Application* application)
		: Widget("Mainmenu"), mApplication(application)
	{
		mImDemo = new ImDemo();
		Renderer::GUI::GetRef().AddWidget(mImDemo);
		
		mConsole = new Console();
		Renderer::GUI::GetRef().AddWidget(mConsole);
		
		mDebugWindow = new DebugWindow(mApplication);
		Renderer::GUI::GetRef().AddWidget(mDebugWindow);
		
		mExplorer = new Explorer(mApplication);
		Renderer::GUI::GetRef().AddWidget(mExplorer);
		
		mPrefabHierarchy = new PrefabHierarchy(application, mExplorer);
		Renderer::GUI::GetRef().AddWidget(mPrefabHierarchy);
		
		mViewport = new Viewport(mPrefabHierarchy);
		Renderer::GUI::GetRef().AddWidget(mViewport);
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
			if (ImGui::Checkbox("Info", &info)) {
				mDebugWindow->SetOpened(info);
			}

			// info box
			bool console = mConsole->IsOpened();
			if (ImGui::Checkbox("Console", &console)) {
				mConsole->SetOpened(console);
			}

			// file explorer
			bool explorer = mExplorer->IsOpened();
			if (ImGui::Checkbox("Explorer", &explorer)) {
				mExplorer->SetOpened(explorer);
			}

			// scene hierarchy
			bool sceneHierarchy = mPrefabHierarchy->IsOpened();
			if (ImGui::Checkbox("Scene Hierarchy", &sceneHierarchy)) {
				mPrefabHierarchy->SetOpened(sceneHierarchy);
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