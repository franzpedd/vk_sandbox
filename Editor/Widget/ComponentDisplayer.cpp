#include "ComponentDisplayer.h"

#include "PrefabHierarchy.h"

#include <Common/Core/Defines.h>
#include <Common/Debug/Logger.h>

#include <Engine/Entity/Entity.h>
#include <Engine/Entity/Components/BaseComponents.h>

#include <Renderer/Core/IMesh.h>
#include <Renderer/Core/ITexture.h>
#include <Renderer/GUI/CustomWidget.h>
#include <Renderer/GUI/GUI.h>
#include <Renderer/GUI/Icon.h>
#include <Renderer/Wrapper/imgui.h>

#include <filesystem>

namespace Cosmos::Editor
{
	ComponentDisplayer::ComponentDisplayer(PrefabHierarchy* prefabHierarchy)
		: mPrefabHierarchy(prefabHierarchy)
	{
	}

	ComponentDisplayer::~ComponentDisplayer()
	{
	}

	void ComponentDisplayer::OnUpdate()
	{
		if (ImGui::Begin(ICON_FA_PUZZLE_PIECE " Components", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_HorizontalScrollbar)) {
			if (ImGui::BeginMenuBar()) {
				if (ImGui::BeginMenu(ICON_FA_PLUS_SQUARE)) {
					AddComponentOnList<Engine::TransformComponent>("Transform", mPrefabHierarchy->GetSelectedEntity());
					AddComponentOnList<Engine::MeshComponent>("Mesh", mPrefabHierarchy->GetSelectedEntity());
				
					ImGui::EndMenu();
				}
		
				ImGui::SameLine();
				ImGui::Text(" Add Component");
		
				ImGui::EndMenuBar();
			}
			
			ImGui::Separator();
			UpdateEntityComponents(mPrefabHierarchy->GetSelectedEntity());
			ImGui::End();
		}
	}

	void ComponentDisplayer::UpdateEntityComponents(Engine::Entity* entity)
	{
		if (entity == nullptr) {
			return;
		}

		ImGui::SeparatorText("General Info");
		{
			ImGui::Text(ICON_LC_HASH " ID: %llu", entity->GetComponent<Engine::IDComponent>().id->GetValue());
			ImGui::Text(ICON_LC_PEN " Name:");
			ImGui::SameLine();

			char buffer[COSMOS_DISPLAY_NAME_MAX_SIZE];
			memset(buffer, 0, sizeof(buffer));
			std::strncpy(buffer, entity->GetComponent<Engine::NameComponent>().name.c_str(), sizeof(buffer));
			
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 1.0f));
			if (ImGui::InputText("##NameTag", buffer, sizeof(buffer))) {
				entity->GetComponent<Engine::NameComponent>().name = std::string(buffer);
			}
			ImGui::PopStyleVar();

			ImGui::Separator();
		}

		ShowComponent<Engine::TransformComponent>("Transform", entity, [&](Engine::TransformComponent& component)
			{
				ImGui::Text("T: ");
				ImGui::SameLine();
				Renderer::CustomWidget::Vector3Control("Translation", component.translation);

				ImGui::Text("R: ");
				ImGui::SameLine();
				glm::vec3 rotation = glm::degrees(component.rotation);
				Renderer::CustomWidget::Vector3Control("Rotation", rotation);
				component.rotation = glm::radians(rotation);

				ImGui::Text("S: ");
				ImGui::SameLine();
				Renderer::CustomWidget::Vector3Control("Scale", component.scale);
			});

		ShowComponent<Engine::MeshComponent>("Mesh", entity, [&](Engine::MeshComponent& component)
			{
				if (component.mesh == nullptr) {
					component.mesh = Renderer::IMesh::Create();
				}

				// path to mesh object on disk
				ImGui::SeparatorText("Path");
				{
					std::filesystem::path path = component.mesh->GetPathRef();
					char buffer[COSMOS_PATH_MAX_SIZE];
					memset(buffer, 0, sizeof(buffer));
					std::strncpy(buffer, path.filename().string().c_str(), sizeof(buffer));

					ImGui::Text(ICON_FA_CUBE);
					ImGui::SameLine();
					
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 1.0f));
					ImGui::InputTextWithHint("##DropMesh", "drag and drop from explorer", buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);
					ImGui::PopStyleVar();

					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EXPLORER")) {
							std::string path = (const char*)payload->Data;
							component.mesh->LoadFromFile(path);
						}
					
						ImGui::EndDragDropTarget();
					}
				}
				
				// material
				ImGui::SeparatorText("Material");
				{
					// albedo
					if (component.mesh->GetMaterialRef().GetAlbedoTextureRef() != nullptr) {

						ImVec2 imagePos = ImGui::GetCursorPos();
						ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 40.0f, ImGui::GetCursorPos().y + 8.0f));
						
						ImGui::Text("Albedo: ");
						ImGui::SameLine();

						if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
							ImGui::SetTooltip("The albedo texture defines the base color of the surface.");
						}
							
						std::filesystem::path path = component.mesh->GetMaterialRef().GetAlbedoTextureRef()->GetPathRef();
						ImGui::SmallButton(path.filename().string().c_str());

						if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
							ImGui::SetTooltip("You may drag and drop a texture from the explorer.");
						}

						if (ImGui::BeginDragDropTarget()) {
							if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EXPLORER")) {
								std::string path = (const char*)payload->Data;
								component.mesh->GetMaterialRef().GetAlbedoTextureRef().reset();
								component.mesh->GetMaterialRef().GetAlbedoTextureRef() = Renderer::ITexture2D::Create(path);
								component.mesh->Refresh();
							}
						
							ImGui::EndDragDropTarget();
						}

						ImGui::SameLine();
						ImGui::SetCursorPos(imagePos);
						ImGui::Image(component.mesh->GetMaterialRef().GetAlbedoTextureRef()->GetUIDescriptor(), ImVec2(32.0f, 32.0f));
					}
				}
			});
	}

	template<typename T>
	inline void ComponentDisplayer::AddComponentOnList(const char* name, Engine::Entity* entity)
	{
		if (entity == nullptr) {
			return;
		}

		if (ImGui::MenuItem(name)) {
			if (!entity->HasComponent<T>()) {
				entity->AddComponent<T>();
				return;
			}
			
			COSMOS_LOG(Logger::Error, "Entity %s already have the component %s", entity->GetComponent<Engine::NameComponent>().name.c_str(), name);
		}
	}

	template<typename T, typename F>
	inline void ComponentDisplayer::ShowComponent(const char* name, Engine::Entity* entity, F function)
	{
		if (entity == nullptr) {
			return;
		}

		if (entity->HasComponent<T>()) {
			auto& component = entity->GetComponent<T>();

			if (ImGui::TreeNodeEx((void*)typeid(T).hash_code(), 0, "%s", name)) {
				if (ImGui::BeginPopupContextItem("##RightClickComponent", ImGuiPopupFlags_MouseButtonRight)) {
					if (ImGui::MenuItem("Remove Component")) {
						entity->RemoveComponent<T>();
					}

					ImGui::EndPopup();
				}

				function(component);
				ImGui::TreePop();
			}
		}
	}
}