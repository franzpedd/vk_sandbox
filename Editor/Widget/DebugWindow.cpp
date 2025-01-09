#include "DebugWindow.h"

#include "Core/Application.h"
#include <Engine/Core/Timestep.h>
#include <Engine/Entity/Camera.h>
#include <Platform/Core/MainWindow.h>
#include <Renderer/Vulkan/Context.h>
#include <Renderer/Vulkan/Swapchain.h>
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
			ImGui::Begin(ICON_FA_INFO_CIRCLE " Debug", nullptr);

			ImGui::Text("App Timestep: %f", mApplication->GetTimestepRef()->GetTimestep());
			ImGui::Text("App FPS: %d", mApplication->GetTimestepRef()->GetFramesPerSecond());
			ImGui::Text("Cam Pos (%.3f %.3f %.3f)", camera.GetViewPos().x, camera.GetViewPos().y, camera.GetViewPos().z);
			ImGui::Text("Cam Rot: (%.3f %.3f %.3f)", camera.GetRotationRef().x, camera.GetRotationRef().y, camera.GetRotationRef().z);
			ImGui::Text("Cam Front: (%.3f %.3f %.3f)", camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);

			ImGui::SeparatorText("Engine");

			Renderer::Vulkan::Context* renderer = (Renderer::Vulkan::Context*)(Renderer::IContext::GetRef());

			
			glm::vec2 framebuffer = Platform::MainWindow::GetRef().GetFrameBufferSize();
			glm::vec2 cursorPos = Platform::MainWindow::GetRef().GetCursorPos();
			VkExtent2D extent = renderer->GetSwapchain()->GetExtent();

			ImGui::Text("Size: (Window: %dx%d):(Swapchain: %dx%d)", framebuffer.x, framebuffer.y, extent.width, extent.height);
			ImGui::Text("Mouse Pos: %lf x %lf", cursorPos.x, cursorPos.y);

			ImGui::End();
		}
	}
}