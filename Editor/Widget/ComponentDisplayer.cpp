#include "ComponentDisplayer.h"

#include "PrefabHierarchy.h"
#include <Common/Debug/Logger.h>
#include <Engine/Entity/Entity.h>
#include <Engine/Entity/Components/BaseComponents.h>
#include <Renderer/GUI/CustomWidget.h>
#include <Renderer/GUI/Icon.h>
#include <Renderer/Wrapper/imgui.h>

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
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu(ICON_FA_PLUS_SQUARE))
				{
					AddComponentOnList<Engine::TransformComponent>("Transform", mPrefabHierarchy->GetSelectedEntity());
				
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

			char buffer[32];
			memset(buffer, 0, sizeof(buffer));
			std::strncpy(buffer, entity->GetComponent<Engine::NameComponent>().name.c_str(), sizeof(buffer));
			
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 1.0f));
			if (ImGui::InputText("##NameTag", buffer, sizeof(buffer)))
			{
				entity->GetComponent<Engine::NameComponent>().name = std::string(buffer);
			}
			ImGui::PopStyleVar();
			ImGui::Separator();
		}

		UpdateComponent<Engine::TransformComponent>("Transform", entity, [&](Engine::TransformComponent& component)
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
	}

	template<typename T>
	inline void ComponentDisplayer::AddComponentOnList(const char* name, Engine::Entity* entity)
	{
		if (entity == nullptr) {
			return;
		}

		if (ImGui::MenuItem(name))
		{
			if (!entity->HasComponent<T>())
			{
				entity->AddComponent<T>();
				return;
			}
			
			COSMOS_LOG(Logger::Error, "Entity %s already have the component %s", entity->GetComponent<Engine::NameComponent>().name.c_str(), name);
		}
	}

	template<typename T, typename F>
	inline void ComponentDisplayer::UpdateComponent(const char* name, Engine::Entity* entity, F function)
	{
		if (entity == nullptr) {
			return;
		}

		if (entity->HasComponent<T>()) {
			auto& component = entity->GetComponent<T>();
			if (ImGui::TreeNodeEx((void*)typeid(T).hash_code(), 0, "%s", name))
			{
				if (ImGui::BeginPopupContextItem("##RightClickComponent", ImGuiPopupFlags_MouseButtonRight))
				{
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