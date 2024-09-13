#include "Gizmos.h"

#include <Common/Math/Math.h>
#include <Engine/Entity/Camera.h>

namespace Cosmos::Editor
{
	void Gizmos::OnUpdate(const std::vector<Shared<Entity>>& entities, ImVec2 viewportSize)
	{
		//if (entities.empty() || mMode == GizmosMode::Undefined) {
		//	return;
		//}
		//
		//ImGuizmo::SetOrthographic(false);
		//ImGuizmo::SetDrawlist();
		//
		//// viewport rect
		//float vpWidth = (float)ImGui::GetWindowWidth();
		//float vpHeight = (float)ImGui::GetWindowHeight();
		//ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, vpWidth, vpHeight);
		//
		//// camera
		//auto& camera = Engine::Camera::GetRef();
		//glm::mat4 view = camera.GetViewRef();
		//glm::mat4 proj = glm::perspectiveRH(glm::radians(camera.GetFov()), viewportSize.x / viewportSize.y, camera.GetNear(), camera.GetFar());
		//
		////// entity
		//// must consider all entities
		////auto& tc = selectedEntity->GetComponent<TransformComponent>();
		////glm::mat4 transform = tc.GetTransform();
		//
		//// snapping
		//float snapValue = mMode == GizmosMode::Rotate ? mSnappingValue + 5.0f : mSnappingValue;
		//float snapValues[3] = { snapValue, snapValue, snapValue };
		//
		//// gizmos drawing
		//ImGuizmo::Manipulate
		//(
		//	glm::value_ptr(view),
		//	glm::value_ptr(proj),
		//	(ImGuizmo::OPERATION)mMode,
		//	ImGuizmo::MODE::LOCAL,
		//	glm::value_ptr(transform),
		//	nullptr,
		//	mSnapping ? snapValues : nullptr
		//);

		//if (ImGuizmo::IsUsing()) // must consider all objects
		//{
		//	glm::vec3 translation, rotation, scale;
		//	Decompose(transform, translation, rotation, scale);
		//
		//	glm::vec3 deltaRotation = rotation - tc.rotation;
		//	tc.translation = translation;
		//	tc.rotation += deltaRotation;
		//	tc.scale = scale;
		//}
	}
}