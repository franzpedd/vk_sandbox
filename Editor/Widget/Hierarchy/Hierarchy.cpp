#include "Hierarchy.h"

#include "ComponentDisplayer.h"
#include "Core/Application.h"
#include <Common/Debug/Logger.h>
#include <Engine/Core/Scene.h>
#include <Engine/Entity/Entity.h>
#include <Engine/Entity/Components/BaseComponents.h>
#include <Platform/Core/Input.h>
#include <Platform/Event/EventBase.h>
#include <Platform/Event/MouseEvent.h>
#include <Renderer/GUI/Icon.h>
#include <Renderer/Wrapper/imgui.h>
#include <set>

namespace Cosmos::Editor
{
	Hierarchy::Hierarchy(Application* application)
		: Widget("Hierarchy"), mApplication(application)
	{
		COSMOS_LOG(Logger::Todo, "Implement multiple-selection context");

		mRoot = new Group();
		mMovingEntity = new EntityMovingRequest();
		mMovingGroup = new GroupMovingRequest();
		mComponentDisplayer = CreateUnique<ComponentDisplayer>();
	}

	Hierarchy::~Hierarchy()
	{
		delete mMovingGroup;
		delete mMovingEntity;
		delete mRoot;
	}

	void Hierarchy::OnUpdate()
	{
		if (mOpened)
		{
			ImGui::Begin("Hierarchy", nullptr);

			DisplayRootMenu();
			DragAndDropTarget(mRoot);
			UpdateGroups(nullptr, mRoot);
			UpdateDeletionQueue();
			
			mComponentDisplayer->OnUpdate(mLastSelectedEntity);

			ImGui::End();
		}
	}

	void Hierarchy::UpdateGroups(Group* parent, Group* current)
	{
		if (current == mRoot) {
			for (auto subgroup : current->subgroups) {
				UpdateGroups(current, subgroup.second);
			}

			for (auto entity : current->entities) {
				UpdateEntity(current, entity.second);
			}
			
			return;
		}

		if (mRenamingGroup == current) {
			char buffer[32];
			memset(buffer, 0, sizeof(buffer));
			std::strncpy(buffer, mRenamingGroup->name.c_str(), sizeof(buffer));

			if (ImGui::InputText("##RenameGroup", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
				auto& range = parent->subgroups.equal_range(mRenamingGroup->name);
				for (auto& it = range.first; it != range.second; ++it) {
					if (it->second == mRenamingGroup) {
						auto x = parent->subgroups.extract(it);
						x.key() = std::string(buffer);
						parent->subgroups.insert(std::move(x));
						mRenamingGroup->name = std::string(buffer);
						mRenamingGroup = nullptr;
						break;
					}
				}
			}

			return;
		}

		if (ImGui::TreeNode((const void*)current, ICON_FA_FOLDER " %s", current->name.c_str()))
		{
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsKeyPressed(ImGuiKey_LeftCtrl)) {
				mLastSelectedEntity = nullptr;
				mRenamingEntity = nullptr;
			}

			DragAndDropTarget(current);
			DragAndDropSource(1, nullptr, parent, current);
			DisplayGroupMenu(parent, current);

			for (auto subgroup : current->subgroups) {
				UpdateGroups(current, subgroup.second);
			}
			
			for (auto entity : current->entities) {
				UpdateEntity(current, entity.second);
			}

			ImGui::TreePop();
		}
	}
	
	void Hierarchy::UpdateEntity(Group* current, Engine::Entity* entity)
	{
		if (entity == nullptr) {
			return;
		}

		if (mRenamingEntity == entity) {
			std::string& nameAux = entity->GetComponent<Engine::NameComponent>().name;
			char buffer[32];
			memset(buffer, 0, sizeof(buffer));
			std::strncpy(buffer, nameAux.c_str(), sizeof(buffer));

			if (ImGui::InputText("##RenameEntity", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
				auto& range = current->entities.equal_range(nameAux);
				for (auto& it = range.first; it != range.second; ++it) {
					if (it->second == mRenamingEntity) {
						auto x = current->entities.extract(it);
						x.key() = std::string(buffer);
						current->entities.insert(std::move(x));
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
			
			DragAndDropSource(0, entity, nullptr, current);
			DisplayEntityMenu(current, entity);
			ImGui::TreePop();
		}
		ImGui::Indent();
	}

	void Hierarchy::UpdateDeletionQueue()
	{
		for (auto& entry : mEntityDeletionQueue) {
			auto& range = entry.current->entities.equal_range(entry.entity->GetComponent<Engine::NameComponent>().name);
			for (auto& it = range.first; it != range.second; ++it) {
				if (it->second == entry.entity) {
					entry.current->entities.erase(it);
					mApplication->GetCurrentScene()->DestroyEntity(entry.entity);
					break;
				}
			}
		}
		mEntityDeletionQueue.clear();

		for (auto& entry : mGroupDeletionQueue) {
			DestroyGroupHierarchy(entry.parent, entry.current);

			auto& range = entry.parent->subgroups.equal_range(entry.current->name);
			for (auto& it = range.first; it != range.second; ++it) {
				if (entry.current == it->second) {
					entry.parent->subgroups.erase(it);
					break;
				}
			}
		}
		mGroupDeletionQueue.clear();
	}

	void Hierarchy::DestroyGroupHierarchy(Group* parent, Group* current)
	{
		if (current == nullptr) {
			return;
		}

		for (auto it = current->subgroups.begin(); it != current->subgroups.end(); it++) {
			DestroyGroupHierarchy(current, it->second);
		}

		for (auto it = current->entities.begin(); it != current->entities.end(); it++) {
			mApplication->GetCurrentScene()->DestroyEntity(it->second);
		}
		current->entities.clear();
	}

	void Hierarchy::DisplayRootMenu()
	{
		if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
			return;
		}

		if (ImGui::BeginPopupContextWindow("##RightClickHierarchyWindow", ImGuiPopupFlags_MouseButtonRight))
		{
			if (ImGui::MenuItem(ICON_LC_PLUS " Create Group")) {
				mRoot->subgroups.insert({ "New Group", new Group() });
			}
		
			ImGui::Separator();
		
			if (ImGui::MenuItem(ICON_LC_PLUS " Create Entity")) {
				mRoot->entities.insert({ "New Entity", mApplication->GetCurrentScene()->CreateEntity("New Entity")});
			}
		
			ImGui::EndPopup();
		}
	}

	void Hierarchy::DisplayGroupMenu(Group* parent, Group* current)
	{
		if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
			return;
		}

		if (ImGui::BeginPopupContextItem("##RightClickHierarchyGroup", ImGuiPopupFlags_MouseButtonRight))
		{
			if (ImGui::MenuItem(ICON_LC_PLUS " Create Group")) {
				current->subgroups.insert({ "New Group", new Group() });
			}
		
			if (ImGui::MenuItem(ICON_LC_PEN_LINE " Rename Group")) {
				mRenamingGroup = current;
			}
		
			if (ImGui::MenuItem(ICON_LC_TRASH " Delete Group")) {
				mGroupDeletionQueue.push_back(GroupDeletionRequest(parent, current));
			}
		
			ImGui::Separator();
		
			if (ImGui::MenuItem(ICON_LC_PLUS " Create Entity")) {
				current->entities.insert({ "New Entity", mApplication->GetCurrentScene()->CreateEntity("New Entity")});
			}
		
			ImGui::EndPopup();
		}
	}

	void Hierarchy::DisplayEntityMenu(Group* current, Engine::Entity* entity)
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
				mEntityDeletionQueue.push_back(EntityDeletionRequest(current, entity));
			}

			ImGui::EndPopup();
		}
	}

	void Hierarchy::DragAndDropSource(int32_t movingType, Engine::Entity* entity, Group* parent, Group* current)
	{
		switch (movingType)
		{
			case 0: // entity
			{
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoPreviewTooltip)) {
					ImGui::BeginTooltip();
					ImGui::Text("Moving %s", entity->GetComponent<Engine::NameComponent>().name.c_str());
					ImGui::EndTooltip();
				
					mMovingEntity->current = current;
					mMovingEntity->entity = entity;
				
					ImGui::SetDragDropPayload("HIERARCHY_ENTITY", mMovingEntity, sizeof(EntityMovingRequest), ImGuiCond_Once);
					ImGui::EndDragDropSource();
				}

				break;
			}

			case 1: // group
			{
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoPreviewTooltip)) {
					ImGui::BeginTooltip();
					ImGui::Text("%s", current->name.c_str());
					ImGui::EndTooltip();

					mMovingGroup->current = current;
					mMovingGroup->parent = parent;

					ImGui::SetDragDropPayload("HIERARCHY_GROUP", mMovingGroup, sizeof(GroupMovingRequest), ImGuiCond_Once);
					ImGui::EndDragDropSource();
				}

				break;
			}

			default: { break; }
		}
	}

	void Hierarchy::DragAndDropTarget(Group* movingTo)
	{
		if (movingTo == mRoot) {
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal | ImGuiSeparatorFlags_SpanAllColumns, 2.0f);
		}

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_GROUP")) {
				GroupMovingRequest* movingGroup = (GroupMovingRequest*)payload->Data;
		
				bool found = false;
				movingGroup->IsValidMove(movingTo, movingGroup->current, &found);
		
				if (!found) {
					auto& range = movingGroup->parent->subgroups.equal_range(movingGroup->current->name);
					for (auto& it = range.first; it != range.second; ++it) {
						if (it->second == movingGroup->current) {
							auto extract = movingGroup->parent->subgroups.extract(it);
							movingTo->subgroups.insert({ movingGroup->current->name, movingGroup->current });
							break;
						}
					}
				}
			}
		
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ENTITY")) {
				EntityMovingRequest* movingEntity = (EntityMovingRequest*)payload->Data;
		
				auto& range = movingEntity->current->entities.equal_range(movingEntity->entity->GetComponent<Engine::NameComponent>().name);
				for (auto& it = range.first; it != range.second; ++it) {
					if (it->second == movingEntity->entity) {
						auto extract = movingEntity->current->entities.extract(it);
						movingTo->entities.insert({ movingEntity->entity->GetComponent<Engine::NameComponent>().name, movingEntity->entity });
						break;
					}
				}
			}
		
			ImGui::EndDragDropTarget();
		}
	}

	void Hierarchy::GroupMovingRequest::IsValidMove(Group* movingTo, Group* current, bool* found)
	{
		if (movingTo == current || *found == true) {
			*found = true;
		}
		
		else {
			for (auto& entry : current->subgroups) {
				IsValidMove(movingTo, entry.second, found);
			}
		}
	}
}