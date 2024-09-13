#include "DebugWindow.h"

#include "Core/Application.h"
#include <Engine/Core/Timestep.h>
#include <Engine/Entity/Camera.h>
#include <Renderer/GUI/Icon.h>
#include <Wrapper/imgui.h>

namespace Cosmos::Editor
{
	DebugWindow::DebugWindow(Application* application)
		: Widget("Debug Window"), mApplication(application)
	{
	}

	void DebugWindow::OnUpdate()
	{
		auto& camera = Engine::Camera::GetRef();

		if (mOpened)
		{
			ImGui::Begin(ICON_FA_INFO_CIRCLE " Debug", &mOpened);

			ImGui::Text("App Timestep: %f", mApplication->GetTimestepRef()->GetTimestep());
			ImGui::Text("App FPS: %.3f", mApplication->GetTimestepRef()->GetFramesPerSecond());
			ImGui::Text("Cam Pos (%.3f %.3f %.3f)", camera.GetPositionRef().x, camera.GetPositionRef().y, camera.GetPositionRef().z);
			ImGui::Text("Cam Rot: (%.3f %.3f %.3f)", camera.GetRotationRef().x, camera.GetRotationRef().y, camera.GetRotationRef().z);
			ImGui::Text("Cam Aim: (%.3f %.3f %.3f)", camera.GetFrontRef().x, camera.GetFrontRef().y, camera.GetFrontRef().z);

			ImGui::End();
		}
	}
}