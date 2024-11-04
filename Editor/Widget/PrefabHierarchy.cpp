#include "PrefabHierarchy.h"

#include "ComponentDisplayer.h"
#include "Explorer.h"
#include "Core/Application.h"
#include <Common/Debug/Logger.h>
#include <Common/File/Datafile.h>
#include <Common/File/Filesystem.h>
#include <Engine/Core/Scene.h>
#include <Engine/Entity/Entity.h>
#include <Engine/Entity/Prefab.h>
#include <Engine/Entity/Components/BaseComponents.h>
#include <Platform/Core/Input.h>
#include <Platform/Event/EventBase.h>
#include <Platform/Event/MouseEvent.h>
#include <Renderer/GUI/Icon.h>
#include <Renderer/Wrapper/imgui.h>
#include <filesystem>
#include <set>

namespace Cosmos::Editor
{
	PrefabHierarchy::PrefabHierarchy(Application* application, Explorer* explorer)
		: Widget("Hierarchy"), mApplication(application), mExplorer(explorer)
	{
		COSMOS_LOG(Logger::Info, "Incorporate Components Displayer into PrefabHierarchy Panel");
		COSMOS_LOG(Logger::Info, "Implement multiple-selection context");

		mComponentDisplayer = CreateUnique<ComponentDisplayer>(this);
	}

	PrefabHierarchy::~PrefabHierarchy()
	{
	}

	void PrefabHierarchy::OnUpdate()
	{
		if (mOpened)
		{
			ImGui::Begin(ICON_FA_LIST " Hierarchy", nullptr);

			DisplayMenubar();

			ImGui::BeginChild(" ##HierarchyChild");
			DisplayRootMenu();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal | ImGuiSeparatorFlags_SpanAllColumns, 2.0f);
			DragAndDropTarget(mApplication->GetCurrentScene()->GetRootPrefab());
			UpdatePrefabs(nullptr, mApplication->GetCurrentScene()->GetRootPrefab());
			UpdateDeletionQueue();
			ImGui::EndChild();
			
			mComponentDisplayer->OnUpdate();
			
			ImGui::End();
		}
	}

	void PrefabHierarchy::UpdatePrefabs(Engine::Prefab* parent, Engine::Prefab* current)
	{
		if (current == mApplication->GetCurrentScene()->GetRootPrefab()) {
			for (auto subgroup : current->GetChildrenRef()) {
				UpdatePrefabs(current, subgroup.second);
			}

			for (auto entity : current->GetEntitiesRef()) {
				UpdateEntity(current, entity.second);
			}
			
			return;
		}

		if (current == mRenamingPrefab) {
			std::string& nameAux = current->GetNameRef();
			char buffer[32];
			memset(buffer, 0, sizeof(buffer));
			std::strncpy(buffer, nameAux.c_str(), sizeof(buffer));

			if (ImGui::InputText("##RenamePrefab", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
				auto range = parent->GetChildrenRef().equal_range(nameAux);
				for (auto& it = range.first; it != range.second; ++it) {
					if (it->second == current) {
						auto x = parent->GetChildrenRef().extract(it);
						x.key() = std::string(buffer);
						parent->GetChildrenRef().insert(std::move(x));
						nameAux = std::string(buffer);
						break;
					}
				}
				mRenamingPrefab = nullptr;
			}

			return;
		}

		if (ImGui::TreeNode((const void*)current, ICON_FA_FOLDER " %s", current->GetNameRef().c_str()))
		{
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsKeyPressed(ImGuiKey_LeftCtrl)) {
				mLastSelectedEntity = nullptr;
				mRenamingEntity = nullptr;
			}

			DragAndDropTarget(current);
			DragAndDropSource(false, nullptr, parent, current);
			DisplayPrefabMenu(parent, current);

			for (auto subgroup : current->GetChildrenRef()) {
				UpdatePrefabs(current, subgroup.second);
			}
			
			for (auto entity : current->GetEntitiesRef()) {
				UpdateEntity(current, entity.second);
			}

			ImGui::TreePop();
		}
	}
	
	void PrefabHierarchy::UpdateEntity(Engine::Prefab* current, Engine::Entity* entity)
	{
		if (entity == nullptr) {
			return;
		}

		if (entity == mRenamingEntity) {
			std::string& nameAux = entity->GetComponent<Engine::NameComponent>().name;
			char buffer[32];
			memset(buffer, 0, sizeof(buffer));
			std::strncpy(buffer, nameAux.c_str(), sizeof(buffer));

			if (ImGui::InputText("##RenameEntity", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
				auto range = current->GetEntitiesRef().equal_range(nameAux);
				for (auto& it = range.first; it != range.second; ++it) {
					if (it->second == entity) {
						auto x = current->GetEntitiesRef().extract(it);
						x.key() = std::string(buffer);
						current->GetEntitiesRef().insert(std::move(x));
						nameAux = std::string(buffer);
						mRenamingEntity = nullptr;
						break;
					}
				}
			}

			return;
		}

		std::string formatedName = {};
		formatedName.append(ICON_FA_FILE_O);
		formatedName.append(" ");
		formatedName.append(entity->GetComponent<Engine::NameComponent>().name);

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;
		if (mLastSelectedEntity == entity) {
			flags |= ImGuiTreeNodeFlags_Selected;
		}

		ImGui::Unindent();
		if (ImGui::TreeNodeEx((const void*)entity->GetComponent<Engine::IDComponent>().id->GetValue(), flags, formatedName.c_str()))
		{
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
				mRenamingEntity = nullptr;
				if (entity == mLastSelectedEntity) {
					mLastSelectedEntity = nullptr;
				}
				
				else {
					mLastSelectedEntity = entity;
				}
			}
			
			DragAndDropSource(true, entity, nullptr, current);
			DisplayEntityMenu(current, entity);
			ImGui::TreePop();
		}
		ImGui::Indent();
	}

	void PrefabHierarchy::UpdateDeletionQueue()
	{
		for (auto& entry : mEntityDeletionQueue) {
			if (entry.entity == mLastSelectedEntity) {
				mLastSelectedEntity = nullptr;
			}

			entry.current->EraseEntity(entry.entity);
		}

		for (auto& entry : mPrefabDeletionQueue) {
			entry.parent->EraseChild(entry.current);
		}

		mEntityDeletionQueue.clear();
		mPrefabDeletionQueue.clear();
	}

	void PrefabHierarchy::DisplayMenubar()
	{
		ImGui::Text(ICON_FA_PAINT_BRUSH " Edit Scene");
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowSize().x - 60.0f);
		
		if (ImGui::Button(ICON_LC_SAVE "##SaveCurrentScene")) {
			Datafile scene = mApplication->GetCurrentScene()->Serialize();
			std::string path = GetAssetSubDir("Scene");
			path.append("/");
			path.append(mApplication->GetCurrentScene()->GetName());
			path.append(".scene");
			Datafile::Write(scene, path);
		}
		ImGui::SetItemTooltip("Save current scene");
		
		ImGui::SameLine();
		
		if (ImGui::Button(ICON_LC_SAVE_ALL "##SaveCurrentSceneAs")) {
			ImGui::OpenPopup("Save current scene as");
		}
		ImGui::SetItemTooltip("Save current scene as");
		
		if (ImGui::BeginPopupModal("Save current scene as", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

			char buffer[32] = {};
			static std::string sSceneName = {};

			if (ImGui::InputTextWithHint("##SavingCurrentSceneAs", "give it a name", buffer, sizeof(buffer))) {
				sSceneName = std::string(buffer);
			}

			std::string path = GetAssetSubDir("Scene");
			path.append("/");
			path.append(sSceneName);
			path.append(".scene");

			if (std::filesystem::exists(path)) {
				ImGui::TextColored(ImVec4(1.0f, 0.1f, 0.1f, 1.0f), "Previous '%s' will be overwritten", sSceneName.c_str());
			}

			if (ImGui::Button("Save ##SaveCurrentSceneAs:Create")) {
				if (!sSceneName.empty()) { // ensures user has given it a name

					mApplication->GetCurrentScene()->SetName(sSceneName);
					Datafile scene = mApplication->GetCurrentScene()->Serialize();
					Datafile::Write(scene, path);

					mExplorer->HintRefresh();
				}

				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel ##SaveCurrentSceneAs:Cancel")) {
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void PrefabHierarchy::DisplayRootMenu()
	{
		if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
			return;
		}

		if (ImGui::BeginPopupContextWindow("##RightClickHierarchyWindow", ImGuiPopupFlags_MouseButtonRight))
		{
			if (ImGui::MenuItem(ICON_LC_PLUS " Create Prefab")) {
				mApplication->GetCurrentScene()->GetRootPrefab()->InsertChild("New Prefab");
			}
		
			ImGui::Separator();
		
			if (ImGui::MenuItem(ICON_LC_PLUS " Create Entity")) {
				mApplication->GetCurrentScene()->GetRootPrefab()->InsertEntity("New Entity");
			}
		
			ImGui::EndPopup();
		}
	}

	void PrefabHierarchy::DisplayPrefabMenu(Engine::Prefab* parent, Engine::Prefab* current)
	{
		if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
			return;
		}

		if (ImGui::BeginPopupContextItem("##RightClickHierarchyPrefab", ImGuiPopupFlags_MouseButtonRight))
		{
			if (ImGui::MenuItem(ICON_LC_PLUS " Create Prefab")) {
				current->InsertChild("New Prefab");
			}
		
			if (ImGui::MenuItem(ICON_LC_PEN_LINE " Rename Prefab")) {
				mRenamingPrefab = current;
			}
		
			if (ImGui::MenuItem(ICON_LC_TRASH " Delete Prefab")) {
				mPrefabDeletionQueue.push_back(PrefabRequest(parent, current));
			}
		
			ImGui::Separator();
		
			if (ImGui::MenuItem(ICON_LC_PLUS " Create Entity")) {
				current->InsertEntity("New Entity");
			}
		
			ImGui::EndPopup();
		}
	}

	void PrefabHierarchy::DisplayEntityMenu(Engine::Prefab* current, Engine::Entity* entity)
	{
		// dont show menus when left ctrl is pressed
		if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
			return;
		}

		if (ImGui::BeginPopupContextItem("##RightClickHierarchyEntity", ImGuiPopupFlags_MouseButtonRight))
		{
			if (ImGui::MenuItem(ICON_LC_PEN_LINE " Rename Entity")) {
				mRenamingEntity = entity;
			}

			if (ImGui::MenuItem(ICON_LC_TRASH " Delete Entity")) {
				mEntityDeletionQueue.push_back(EntityRequest(current, entity));
			}

			ImGui::EndPopup();
		}
	}

	void PrefabHierarchy::DragAndDropSource(bool isEntity, Engine::Entity* entity, Engine::Prefab* parent, Engine::Prefab* current)
	{
		if (isEntity) {
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoPreviewTooltip | ImGuiDragDropFlags_SourceAllowNullID)) {
				ImGui::BeginTooltip();
				ImGui::Text("Moving %s", entity->GetComponent<Engine::NameComponent>().name.c_str());
				ImGui::EndTooltip();

				mMovingEntity.current = current;
				mMovingEntity.entity = entity;

				ImGui::SetDragDropPayload("HIERARCHY_ENTITY", &mMovingEntity, sizeof(EntityRequest), ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}
			return;
		}
		
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoPreviewTooltip | ImGuiDragDropFlags_SourceAllowNullID)) {
			ImGui::BeginTooltip();
			ImGui::Text("%s", current->GetNameRef().c_str());
			ImGui::EndTooltip();
		
			mMovingPrefab.current = current;
			mMovingPrefab.parent = parent;
		
			ImGui::SetDragDropPayload("HIERARCHY_PREFAB", &mMovingPrefab, sizeof(PrefabRequest), ImGuiCond_Once);
			ImGui::EndDragDropSource();
		}
	}

	void PrefabHierarchy::DragAndDropTarget(Engine::Prefab* movingTo)
	{
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_PREFAB")) {
				PrefabRequest* movingPrefab = (PrefabRequest*)payload->Data;
		
				bool found = false;
				movingPrefab->IsValidMove(movingTo, movingPrefab->current, &found);
		
				if (!found) {
					auto range = movingPrefab->parent->GetChildrenRef().equal_range(movingPrefab->current->GetNameRef());
					for (auto& it = range.first; it != range.second; ++it) {
						if (it->second == movingPrefab->current) {
							auto extract = movingPrefab->parent->GetChildrenRef().extract(it);
							movingTo->GetChildrenRef().insert({movingPrefab->current->GetNameRef(), movingPrefab->current});
							break;
						}
					}
				}
			}
		
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ENTITY")) {
				EntityRequest* movingEntity = (EntityRequest*)payload->Data;
		
				auto range = movingEntity->current->GetEntitiesRef().equal_range(movingEntity->entity->GetComponent<Engine::NameComponent>().name);
				for (auto& it = range.first; it != range.second; ++it) {
					if (it->second == movingEntity->entity) {
						auto extract = movingEntity->current->GetEntitiesRef().extract(it);
						movingTo->GetEntitiesRef().insert({movingEntity->entity->GetComponent<Engine::NameComponent>().name, movingEntity->entity});
						break;
					}
				}
			}
		
			ImGui::EndDragDropTarget();
		}
	}

	void PrefabHierarchy::PrefabRequest::IsValidMove(Engine::Prefab* movingTo, Engine::Prefab* current, bool* found)
	{
		if (movingTo == current || *found == true) {
			*found = true;
			return;
		}
		
		for (auto& entry : current->GetChildrenRef()) {
			IsValidMove(movingTo, entry.second, found);
		}
	}
}