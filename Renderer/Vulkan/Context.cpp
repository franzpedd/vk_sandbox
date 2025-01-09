#include "Context.h"

#include "Buffer.h"
#include "Context.h"
#include "Device.h"
#include "GUI.h"
#include "Instance.h"
#include "Picking.h"
#include "Pipeline.h"
#include "Renderpass.h"
#include "Shader.h"
#include "Swapchain.h"

#include "Core/IGUI.h"
#include "Core/IMesh.h"

#include <Common/Debug/Logger.h>
#include <Common/Debug/Profiler.h>
#include <Common/File/Filesystem.h>
#include <Engine/Core/Application.h>
#include <Engine/Core/Project.h>
#include <Engine/Core/Scene.h>
#include <Engine/Entity/Camera.h>
#include <Platform/Core/MainWindow.h>
#include <Platform/Event/WindowEvent.h>

#include <vector>

namespace Cosmos::Renderer::Vulkan
{
	Context::Context(Engine::Application* application)
	{
		mApplication = application;
		
		auto& settings = mApplication->GetProjectRef()->GetSettingsRef();
		mViewportBoundaries.size = { settings.width, settings.height };

		mInstance = CreateShared<Vulkan::Instance>(settings.enginename, settings.gamename, settings.validations, settings.version, settings.vulkanversion);
		mDevice = CreateShared<Vulkan::Device>(mInstance, 2);
		mSwapchain = CreateShared<Vulkan::Swapchain>(mDevice, mRenderpasses);
		mPicking = CreateShared<Vulkan::Picking>(mApplication, mDevice, mSwapchain, mRenderpasses);

		mMainRenderpass = mRenderpasses.GetRef("Swapchain");
		mBuffers.Insert("Camera", CreateShared<Vulkan::Buffer>(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(Vulkan::CameraBuffer)));

		Vulkan::DefaultPipelinesCreateInfo ci = { mDevice, mMainRenderpass, mPipelines, mRenderpasses };
		Vulkan::CreateDefaultPipelines(ci);
	}

	void Context::OnUpdate()
	{
		// send data to gpu
		{
			PROFILER_SCOPE("Send Data");
		
			// camera data
			glm::vec2 mousepos = Platform::MainWindow::GetRef().GetCursorPos();
			Engine::Camera& cameraObject = Engine::Camera::GetRef();
		
			Vulkan::CameraBuffer camera = {};
			camera.mousepos = glm::vec2(mousepos.x, mousepos.y);
			camera.view = cameraObject.GetViewRef();
			camera.projection = cameraObject.GetProjectionRef();
			memcpy(mBuffers.GetRef("Camera")->GetMappedDataRef()[mCurrentFrame], &camera, sizeof(camera));
		}

		// get next image available from the swapchain
		{
			PROFILER_SCOPE("Aquire Image");
		
			vkWaitForFences(mDevice->GetLogicalDevice(), 1, &mSwapchain->GetInFlightFencesRef()[mCurrentFrame], VK_TRUE, UINT64_MAX);
			VkResult res = vkAcquireNextImageKHR(mDevice->GetLogicalDevice(), mSwapchain->GetSwapchain(), UINT64_MAX, mSwapchain->GetAvailableSemaphoresRef()[mCurrentFrame], VK_NULL_HANDLE, &mSwapchain->GetImageIndexRef());
		
			if (res == VK_ERROR_OUT_OF_DATE_KHR) {
				mSwapchain->Recreate();
				return;
			}
		
			else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
				COSMOS_ASSERT(false, "Failed to acquired next swapchain image");
			}
		
			vkResetFences(mDevice->GetLogicalDevice(), 1, &mSwapchain->GetInFlightFencesRef()[mCurrentFrame]);
		}

		// manage render passes
		{
			ManageRenderpasses(mSwapchain->GetImageIndexRef());
		}
		
		// submits command buffers
		VkSwapchainKHR swapChains[] = { mSwapchain->GetSwapchain() };
		VkSemaphore waitSemaphores[] = { mSwapchain->GetAvailableSemaphoresRef()[mCurrentFrame] };
		VkSemaphore signalSemaphores[] = { mSwapchain->GetFinishedSempahoresRef()[mCurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		{
			PROFILER_SCOPE("Submit");
		
			std::vector<VkCommandBuffer> submitCommandBuffers = { mRenderpasses.GetRef("Swapchain")->GetCommandfuffersRef()[mCurrentFrame] };
		
			if (mRenderpasses.Exists("Picking")) {
				submitCommandBuffers.push_back(mRenderpasses.GetRef("Picking")->GetCommandfuffersRef()[mCurrentFrame]);
			}
		
			if (mRenderpasses.Exists("Viewport")) {
				submitCommandBuffers.push_back(mRenderpasses.GetRef("Viewport")->GetCommandfuffersRef()[mCurrentFrame]);
			}
		
			if (mRenderpasses.Exists("UI")) {
				submitCommandBuffers.push_back(mRenderpasses.GetRef("UI")->GetCommandfuffersRef()[mCurrentFrame]);
			}
		
			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.pNext = nullptr;
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;
			submitInfo.commandBufferCount = (uint32_t)submitCommandBuffers.size();
			submitInfo.pCommandBuffers = submitCommandBuffers.data();
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;
			COSMOS_ASSERT(vkQueueSubmit(mDevice->GetGraphicsQueue(), 1, &submitInfo, mSwapchain->GetInFlightFencesRef()[mCurrentFrame]) == VK_SUCCESS, "Failed to submit draw command");
		}

		
		// presents the image
		{
			PROFILER_SCOPE("Present");

			VkPresentInfoKHR presentInfo = {};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores;
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;
			presentInfo.pImageIndices = &mSwapchain->GetImageIndexRef();

			VkResult res = vkQueuePresentKHR(mDevice->GetPresentQueue(), &presentInfo);

			if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || Platform::MainWindow::GetRef().ShouldResize()) {
				mSwapchain->Recreate();

				Engine::Camera::GetRef().SetAspectRatio(Platform::MainWindow::GetRef().GetAspectRatio());

				Vulkan::GUI* gui = (Vulkan::GUI*)IGUI::GetRef();
				gui->SetImageCount(mSwapchain->GetImageCount());

				VkExtent2D extent = mSwapchain->GetExtent();
				mViewportBoundaries.size = { extent.width, extent.height };

				Shared<Platform::WindowResizeEvent> event = CreateShared<Platform::WindowResizeEvent>((int32_t)extent.width, (int32_t)extent.height);
				mApplication->OnEvent(event);
			}

			else if (res != VK_SUCCESS) {
				COSMOS_ASSERT(false, "Failed to present swapchain image");
			}
		}
	}

	void Context::OnEvent(Shared<Platform::EventBase> event)
	{
		mPicking->OnEvent(event);
	}

	void Context::ManageRenderpasses(uint32_t swapchainImageIndex)
	{
		std::vector<VkClearValue> clearValues(2);
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		// swapchain render pass is guaranteed to exists
		{
			VkCommandBuffer& cmdBuffer = mRenderpasses.GetRef("Swapchain")->GetCommandfuffersRef()[mCurrentFrame];
			VkFramebuffer& frameBuffer = mRenderpasses.GetRef("Swapchain")->GetFramebuffersRef()[swapchainImageIndex];
			VkRenderPass& renderPass = mRenderpasses.GetRef("Swapchain")->GetRenderpassRef();

			vkResetCommandBuffer(cmdBuffer, /*VkCommandBufferResetFlagBits*/ 0);

			VkCommandBufferBeginInfo cmdBeginInfo = {};
			cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmdBeginInfo.pNext = nullptr;
			cmdBeginInfo.flags = 0;
			COSMOS_ASSERT(vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo) == VK_SUCCESS, "Failed to begin command buffer recording");

			VkRenderPassBeginInfo renderPassBeginInfo = {};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass = renderPass;
			renderPassBeginInfo.framebuffer = frameBuffer;
			renderPassBeginInfo.renderArea.offset = { 0, 0 };
			renderPassBeginInfo.renderArea.extent = mSwapchain->GetExtent();
			renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
			renderPassBeginInfo.pClearValues = clearValues.data();
			vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			// set frame commandbuffer viewport
			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)mSwapchain->GetExtent().width;
			viewport.height = (float)mSwapchain->GetExtent().height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

			// set frame commandbuffer scissor
			VkRect2D scissor = {};
			scissor.offset = { 0, 0 };
			scissor.extent = mSwapchain->GetExtent();
			vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

			// if we have a viewport, don't draw on swapchain
			if (!mRenderpasses.Exists("Viewport")) {

				// render objects
				mApplication->OnRender(IContext::Stage::Default);

				// render ui 
				IGUI::GetRef()->OnRender();
			}

			vkCmdEndRenderPass(cmdBuffer);

			// end command buffer
			COSMOS_ASSERT(vkEndCommandBuffer(cmdBuffer) == VK_SUCCESS, "Failed to end command buffer recording");
		}

		// picking
		mPicking->ManageRenderpass(mCurrentFrame, swapchainImageIndex);

		// if we have a viewport we should render it before the rest of the user interface, or it'll bring the interface into the viewport (not a good thing)
		if (mRenderpasses.Exists("Viewport"))
		{
			VkCommandBuffer& cmdBuffer = mRenderpasses.GetRef("Viewport")->GetCommandfuffersRef()[mCurrentFrame];
			VkFramebuffer& frameBuffer = mRenderpasses.GetRef("Viewport")->GetFramebuffersRef()[swapchainImageIndex];
			VkRenderPass& renderPass = mRenderpasses.GetRef("Viewport")->GetRenderpassRef();

			vkResetCommandBuffer(cmdBuffer, /*VkCommandBufferResetFlagBits*/ 0);

			VkCommandBufferBeginInfo cmdBeginInfo = {};
			cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmdBeginInfo.pNext = nullptr;
			cmdBeginInfo.flags = 0;
			COSMOS_ASSERT(vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo) == VK_SUCCESS, "Failed to begin command buffer recording");

			VkRenderPassBeginInfo renderPassBeginInfo = {};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass = renderPass;
			renderPassBeginInfo.framebuffer = frameBuffer;
			renderPassBeginInfo.renderArea.offset = { 0, 0 };
			renderPassBeginInfo.renderArea.extent = mSwapchain->GetExtent();
			renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
			renderPassBeginInfo.pClearValues = clearValues.data();
			vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			// set frame commandbuffer viewport
			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)mSwapchain->GetExtent().width;
			viewport.height = (float)mSwapchain->GetExtent().height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

			// set frame commandbuffer scissor
			VkRect2D scissor = {};
			scissor.offset = { 0, 0 };
			scissor.extent = mSwapchain->GetExtent();
			vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

			// render objects
			mApplication->OnRender(IContext::Stage::Default);

			// render ui 
			IGUI::GetRef()->OnRender();

			vkCmdEndRenderPass(cmdBuffer);

			// end command buffer
			COSMOS_ASSERT(vkEndCommandBuffer(cmdBuffer) == VK_SUCCESS, "Failed to end command buffer recording");
		}

		// if we have a user interface render pass we must render it after all other render passes, so it'll be on top of the rest
		if (mRenderpasses.Exists("UI"))
		{
			VkCommandBuffer& cmdBuffer = mRenderpasses.GetRef("UI")->GetCommandfuffersRef()[mCurrentFrame];
			VkFramebuffer& frameBuffer = mRenderpasses.GetRef("UI")->GetFramebuffersRef()[swapchainImageIndex];
			VkRenderPass& renderPass = mRenderpasses.GetRef("UI")->GetRenderpassRef();

			vkResetCommandBuffer(cmdBuffer, /*VkCommandBufferResetFlagBits*/ 0);

			VkCommandBufferBeginInfo cmdBeginInfo = {};
			cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmdBeginInfo.pNext = nullptr;
			cmdBeginInfo.flags = 0;
			COSMOS_ASSERT(vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo) == VK_SUCCESS, "Failed to begin command buffer recording");

			VkClearValue clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };

			VkRenderPassBeginInfo renderPassBeginInfo = {};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass = renderPass;
			renderPassBeginInfo.framebuffer = frameBuffer;
			renderPassBeginInfo.renderArea.offset = { 0, 0 };
			renderPassBeginInfo.renderArea.extent = mSwapchain->GetExtent();
			renderPassBeginInfo.clearValueCount = 1;
			renderPassBeginInfo.pClearValues = &clearValue;
			vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			// render ui objects
			Vulkan::GUI* gui = (Vulkan::GUI*)IGUI::GetRef();
			gui->DrawBackendData(cmdBuffer);

			vkCmdEndRenderPass(cmdBuffer);

			COSMOS_ASSERT(vkEndCommandBuffer(cmdBuffer) == VK_SUCCESS, "Failed to end command buffer recording");
		}
	}
}