#include "Mainmenu.h"

#include "DebugWindow.h"
#include "Explorer.h"
#include "ImDemo.h"
#include "Core/Application.h"
#include "Hierarchy/Hierarchy.h"
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

		HandleMenuAction();
	}

	void Mainmenu::DisplayMenuItems()
	{
		mMenuAction = MenuAction::None;

		if (ImGui::BeginMenu(ICON_FA_GLOBE " World"))
		{
			ImGui::SeparatorText("Scene");

			if (ImGui::MenuItem(ICON_LC_FILE_PLUS_2 " New")) {
				mMenuAction = SceneNew;
			}

			if (ImGui::MenuItem(ICON_LC_PACKAGE_OPEN " Open")) {
				mMenuAction = SceneOpen;
			}

			if (ImGui::MenuItem(ICON_LC_SAVE " Save")) {
				mMenuAction = SceneSave;
			}

			if (ImGui::MenuItem(ICON_LC_SAVE_ALL " Save As")) {
				mMenuAction = SceneSaveAs;
			}

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
			if (ImGui::Checkbox("Info", &info)) {
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

	void Mainmenu::HandleMenuAction()
	{
		switch (mMenuAction)
		{
			case Cosmos::Editor::Mainmenu::None: { break; }

			case Cosmos::Editor::Mainmenu::SceneNew:
			{
				ImGui::OpenPopup("Create New Scene");
				break;
			}

			case Cosmos::Editor::Mainmenu::SceneOpen:
			{
				ImGui::OpenPopup("Open Scene");
				break;
			}

			case Cosmos::Editor::Mainmenu::SceneSave:
			{
				ImGui::OpenPopup("Save Scene");
				break;
			}

			case Cosmos::Editor::Mainmenu::SceneSaveAs:
			{
				ImGui::OpenPopup("Save Scene As");
				break;
			}

			default: { break; }
		}

		// handle popups
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("Create New Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			// we have default scene currently loaded, just close the pop-up
			if (Engine::Scene::IsDefaultScene(mApplication->GetCurrentScene()->GetSceneDataRef())) {
				ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
				return;
			}

			// we're editing a custom scene, must properly prompt the new scene options
			ImGui::Text("Save current scene '%s' ?", mApplication->GetCurrentScene()->GetName().c_str());
			
			// this will overwrite the scene on disk
			ImGui::PushID("CreateNewSceneYesButton");
			if (ImGui::Button("Yes", ImVec2(120, 0))) {
			
				COSMOS_LOG(Logger::Info, "Saving scene '%s'", mApplication->GetCurrentScene()->GetName().c_str());
				Datafile scene = mApplication->GetCurrentScene()->Serealize();
				
				std::string path = GetAssetSubDir("Scene");
				path.append("/");
				path.append(scene["Name"].GetString());
				path.append(".scene");
				
				Datafile::Write(scene, path);
				mApplication->GetCurrentScene()->Deserialize(Engine::Scene::CreateDefaultScene());
				ImGui::CloseCurrentPopup();
			}
			ImGui::PopID();
			
			ImGui::SameLine();
			
			// we must deserialize the default scene into the current scene
			ImGui::PushID("CreateNewSceneNoButton");
			if (ImGui::Button("No", ImVec2(120, 0))) { 
				COSMOS_LOG(Logger::Info, "Loading default scene");
				mApplication->GetCurrentScene()->Deserialize(Engine::Scene::CreateDefaultScene());
				ImGui::CloseCurrentPopup();
			}
			ImGui::PopID();
			
			ImGui::SameLine();
			
			// just close the pop-up
			ImGui::PushID("CreateNewSceneCancelButton");
			if (ImGui::Button("Cancel", ImVec2(120, 0))) { 
				ImGui::CloseCurrentPopup();
			}
			ImGui::PopID();

			ImGui::EndPopup();
		}

		if (ImGui::BeginPopupModal("Open Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			
			// display input place to type the scene's name
			static std::string openingSceneStr = {};
			static char openingBuffer[32] = {};
			
			if (ImGui::InputTextWithHint("##OpeningSceneInput", "Type name name of the scene here", openingBuffer, sizeof(openingBuffer))) {
				openingSceneStr = std::string(openingBuffer);
			}
			
			// check if scene exists on the Scene folder
			std::string path = GetAssetSubDir("Scene");
			path.append("/");
			path.append(openingSceneStr);
			path.append(".scene");
			
			if (std::filesystem::exists(path)) {
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "'%s' found", path.c_str());
				
				ImGui::PushID("OpenSceneOpenButton");
				if (ImGui::Button("Open", ImVec2(120, 0))) {
					Datafile scene;
					Datafile::Read(scene, path);
					COSMOS_LOG(Logger::Trace, "Opening Scene '%s'", path.c_str());
					mApplication->GetCurrentScene()->Deserialize(scene);
					memset(openingBuffer, 0, sizeof(openingBuffer));
					openingSceneStr.clear();
					ImGui::CloseCurrentPopup();
				}
				ImGui::PopID();
			}
			
			else {
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "'%s' wasn't found", path.c_str());
				
				ImGui::PushID("OpenSceneDisabledOpenButton");
				ImGui::BeginDisabled();
				ImGui::Button("Open", ImVec2(120, 0));
				ImGui::EndDisabled();
				ImGui::PopID();
			}
			
			ImGui::SameLine();
			
			// just close the pop-up
			ImGui::PushID("OpenSceneCancelButton");
			if (ImGui::Button("Cancel", ImVec2(120, 0))) {
				memset(openingBuffer, 0, sizeof(openingBuffer));
				openingSceneStr.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::PopID();
			
			ImGui::EndPopup();
		}

		if (ImGui::BeginPopupModal("Save Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		
			auto& currentScene = mApplication->GetCurrentScene();

			// if scene's name is not Default we should just overwrite whatever we have on disk
			if (currentScene->GetName().compare("Default") != 0) {
				std::string path = GetAssetSubDir("Scene");
				path.append("/");
				path.append(currentScene->GetName());
				path.append(".scene");

				Datafile::Write(currentScene->GetSceneDataRef(), path);
				ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
				return;
			}
		
			// we must ask the user to give us a scene name
			static std::string savingSceneStr = {};
			static char savingBuffer[32] = {};
			
			if (ImGui::InputTextWithHint("##SaveSceneInput", "Type name name of the scene here", savingBuffer, sizeof(savingBuffer))) {
				savingSceneStr = std::string(savingBuffer);
			}
		
			std::string path = GetAssetSubDir("Scene");
			path.append("/");
			path.append(savingSceneStr);
			path.append(".scene");
		
			if (std::filesystem::exists(path)) {
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Scene '%s' already exists and will be overwritten", path.c_str());
			}
			
			// save the current scene
			ImGui::PushID("SaveSceneSaveButton");
			if (ImGui::Button("Save", ImVec2(120, 0))) {
				currentScene->SetName(savingSceneStr);
				Datafile::Write(currentScene->GetSceneDataRef(), path);
				memset(savingBuffer, 0, sizeof(savingBuffer));
				savingSceneStr.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::PopID();
		
			ImGui::SameLine();
			
			// just close the pop-up
			ImGui::PushID("SaveSceneCancelButton");
			if (ImGui::Button("Cancel", ImVec2(120, 0))) {
				memset(savingBuffer, 0, sizeof(savingBuffer));
				savingSceneStr.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::PopID();
		
			ImGui::EndPopup();
		}

		if (ImGui::BeginPopupModal("Save Scene As", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

			auto& currentScene = mApplication->GetCurrentScene();
			// we must ask the user to give us a scene name
			static std::string savingAsSceneAsStr = {};
			static char savingAsBuffer[32] = {};
			
			if (ImGui::InputTextWithHint("##SaveSceneAsInput", "Type name name of the scene here", savingAsBuffer, sizeof(savingAsBuffer))) {
				savingAsSceneAsStr = std::string(savingAsBuffer);
			}
			
			std::string path = GetAssetSubDir("Scene");
			path.append("/");
			path.append(savingAsSceneAsStr);
			path.append(".scene");
			
			if (std::filesystem::exists(path)) {
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Scene '%s' already exists and will be overwritten", path.c_str());
			}
			
			// save the current scene
			ImGui::PushID("SaveSceneAsSaveButton");
			if (ImGui::Button("Save", ImVec2(120, 0))) {
				currentScene->SetName(savingAsSceneAsStr);
				Datafile::Write(currentScene->GetSceneDataRef(), path);
				memset(savingAsBuffer, 0, sizeof(savingAsBuffer));
				savingAsSceneAsStr.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::PopID();
			
			ImGui::SameLine();
			
			// just close the pop-up
			ImGui::PushID("SaveSceneAsCancelButton");
			if (ImGui::Button("Cancel", ImVec2(120, 0))) {
				memset(savingAsBuffer, 0, sizeof(savingAsBuffer));
				savingAsSceneAsStr.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::PopID();
			
			ImGui::EndPopup();
		}
	}
}