#include "Context.h"
#if defined RENDERER_VULKAN

#include "GUI/GUI.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/Device.h"
#include "Vulkan/Instance.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/Renderpass.h"
#include "Vulkan/Shader.h"
#include "Vulkan/Swapchain.h"

#include <Common/Debug/Logger.h>
#include <Common/File/Filesystem.h>

#include <Engine/Core/Application.h>
#include <Engine/Core/Project.h>
#include <Engine/Entity/Camera.h>

#include <Platform/Core/MainWindow.h>
#include <Platform/Event/WindowEvent.h>

namespace Cosmos::Renderer
{
	static Context* s_Instance = nullptr;
	static VkPipelineCache s_PipelineCache = nullptr;

	void Context::Initialize(Engine::Application* application)
	{
		if (s_Instance) {
			COSMOS_LOG(Logger::Warn, "Attempting to initialize Renderer when it's already initialized\n");
			return;
		}

		s_Instance = new Context(application);
	}

	void Context::Shutdown()
	{
		delete s_Instance;
		s_Instance = nullptr;
	}

	Context& Context::GetRef()
	{
		if (!s_Instance) {
			COSMOS_LOG(Logger::Error, "Renderer has not been initialized\n");
		}

		return *s_Instance;
	}

	Context::Context(Engine::Application* application)
		: mApplication(application)
	{
		auto& settings = mApplication->GetProjectRef()->GetSettingsRef();
		mInstance = CreateShared<Vulkan::Instance>(settings.enginename, settings.gamename, settings.validations, settings.version, settings.vulkanversion);
		mDevice = CreateShared<Vulkan::Device>(mInstance, 2);
		mSwapchain = CreateShared<Vulkan::Swapchain>(mDevice, mRenderpasses);
		
		mMainRenderpass = mRenderpasses.GetRef("Swapchain");
		mBuffers.Insert("Camera", CreateShared<Vulkan::Buffer>(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(Vulkan::CameraBuffer)));
		
		RecreatePipelines();
	}

	Context::~Context()
	{
		if (s_PipelineCache != VK_NULL_HANDLE) {
			vkDestroyPipelineCache(mDevice->GetLogicalDevice(), s_PipelineCache, nullptr);
		}
	}

	void Context::OnUpdate()
	{
		// send data to gpu
		{
			// camera data
			Engine::Camera& cameraObject = Engine::Camera::GetRef();
			Vulkan::CameraBuffer camera = {};
			camera.view = cameraObject.GetViewRef();
			camera.projection = cameraObject.GetProjectionRef();
			camera.viewProjection = cameraObject.GetViewRef() * cameraObject.GetProjectionRef();
			camera.cameraFront = cameraObject.GetFrontRef();
			memcpy(mBuffers.GetRef("Camera")->GetMappedDataRef()[mCurrentFrame], &camera, sizeof(camera));
		}

		// render frame
		{
			VkResult res = VK_SUCCESS;

			// aquire image from swapchain
			vkWaitForFences(mDevice->GetLogicalDevice(), 1, &mSwapchain->GetInFlightFencesRef()[mCurrentFrame], VK_TRUE, UINT64_MAX);
			res = vkAcquireNextImageKHR(mDevice->GetLogicalDevice(), mSwapchain->GetSwapchain(), UINT64_MAX, mSwapchain->GetAvailableSemaphoresRef()[mCurrentFrame], VK_NULL_HANDLE, &mImageIndex);

			if (res == VK_ERROR_OUT_OF_DATE_KHR) {
				mSwapchain->Recreate();
				return;
			}

			else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
				COSMOS_ASSERT(false, "Failed to acquired next swapchain image");
			}

			vkResetFences(mDevice->GetLogicalDevice(), 1, &mSwapchain->GetInFlightFencesRef()[mCurrentFrame]);

			// execute the render passes
			ManageRenderpasses(mImageIndex);

			// submits the draw calls
			VkSwapchainKHR swapChains[] = { mSwapchain->GetSwapchain() };
			VkSemaphore waitSemaphores[] = { mSwapchain->GetAvailableSemaphoresRef()[mCurrentFrame] };
			VkSemaphore signalSemaphores[] = { mSwapchain->GetFinishedSempahoresRef()[mCurrentFrame] };
			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			std::vector<VkCommandBuffer> submitCommandBuffers = {
				mRenderpasses.GetRef("Swapchain")->GetCommandfuffersRef()[mCurrentFrame]
			};

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

			// presents the image
			VkPresentInfoKHR presentInfo = {};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores;
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;
			presentInfo.pImageIndices = &mImageIndex;

			res = vkQueuePresentKHR(mDevice->GetPresentQueue(), &presentInfo);

			if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || Platform::MainWindow::GetRef().ShouldResize())
			{
				mSwapchain->Recreate();

				Engine::Camera::GetRef().SetAspectRatio(Platform::MainWindow::GetRef().GetAspectRatio());
				GUI::GetRef().SetImageCount(mSwapchain->GetImageCount());

				COSMOS_LOG(Logger::Trace, "Check if this event is neccessary");
				int32_t width = (int32_t)mSwapchain->GetExtent().width;
				int32_t height = (int32_t)mSwapchain->GetExtent().height;
				
				Shared<Platform::WindowResizeEvent> event = CreateShared<Platform::WindowResizeEvent>(width, height);
				mApplication->OnEvent(event);
			}

			else if (res != VK_SUCCESS) {
				COSMOS_ASSERT(false, "Failed to present swapchain image");
			}
		}
	}

	void Context::RecreatePipelines()
	{
		// cache
		{
			if (s_PipelineCache != VK_NULL_HANDLE) {
				vkDestroyPipelineCache(mDevice->GetLogicalDevice(), s_PipelineCache, nullptr);
			}

			VkPipelineCacheCreateInfo pipelineCacheCI = {};
			pipelineCacheCI.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
			pipelineCacheCI.pNext = nullptr;
			pipelineCacheCI.flags = 0;
			COSMOS_ASSERT(vkCreatePipelineCache(mDevice->GetLogicalDevice(), &pipelineCacheCI, nullptr, &s_PipelineCache) == VK_SUCCESS, "Failed to create pipeline cache");
		}
		
		// meshes
		{
			if (mPipelines.Exists("Mesh")) {
				mPipelines.Erase("Mesh");
			}

			Vulkan::Pipeline::CreateInfo meshSpecification = {};
			meshSpecification.renderPass = mMainRenderpass;
			meshSpecification.vertexShader = CreateShared<Vulkan::Shader>(mDevice, Vulkan::ShaderType::Vertex, "Mesh.vert", GetAssetSubDir("Shader/mesh.vert").c_str());
			meshSpecification.fragmentShader = CreateShared<Vulkan::Shader>(mDevice, Vulkan::ShaderType::Fragment, "Mesh.frag", GetAssetSubDir("Shader/mesh.frag").c_str());
			meshSpecification.vertexComponents =
			{
				Vertex::Component::POSITION,
				Vertex::Component::NORMAL,
				Vertex::Component::UV
			};

			// drawable push constants
			VkPushConstantRange pushConstant = {};
			pushConstant.offset = 0;
			pushConstant.size = sizeof(Vulkan::CameraBuffer);
			pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
			meshSpecification.pushConstants.push_back(pushConstant);

			meshSpecification.bindings.resize(2);

			// camera ubo
			meshSpecification.bindings[0].binding = 0;
			meshSpecification.bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			meshSpecification.bindings[0].descriptorCount = 1;
			meshSpecification.bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			meshSpecification.bindings[0].pImmutableSamplers = nullptr;

			// color map
			meshSpecification.bindings[1].binding = 4;
			meshSpecification.bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			meshSpecification.bindings[1].descriptorCount = 1;
			meshSpecification.bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			meshSpecification.bindings[1].pImmutableSamplers = nullptr;

			// create
			mPipelines.Insert("Mesh", CreateShared<Vulkan::Pipeline>(mDevice, meshSpecification, s_PipelineCache));
			mPipelines.GetRef("Mesh")->GetCreateInfoRef().RSCI.cullMode = VK_CULL_MODE_BACK_BIT;
			mPipelines.GetRef("Mesh")->Build();
		}
		
		COSMOS_LOG(Logger::Todo, "Create Wireframe pipeline");

		// skybox
		{
			// remove previously 
			if (mPipelines.Exists("Skybox")) {
				mPipelines.Erase("Skybox");
			}

			Vulkan::Pipeline::CreateInfo skyboxSpecification = {};
			skyboxSpecification.renderPass = mMainRenderpass;
			skyboxSpecification.vertexShader = CreateShared<Vulkan::Shader>(mDevice, Vulkan::ShaderType::Vertex, "Skybox.vert", GetAssetSubDir("Shader/skybox.vert").c_str());
			skyboxSpecification.fragmentShader = CreateShared<Vulkan::Shader>(mDevice, Vulkan::ShaderType::Fragment, "Skybox.frag", GetAssetSubDir("Shader/skybox.frag").c_str());
			skyboxSpecification.vertexComponents =
			{
				Vertex::Component::POSITION
			};

			// drawable push constants
			VkPushConstantRange pushConstant = {};
			pushConstant.offset = 0;
			pushConstant.size = sizeof(Vulkan::CameraBuffer);
			pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			skyboxSpecification.pushConstants.push_back(pushConstant);

			skyboxSpecification.bindings.resize(2);
			// global ubo
			skyboxSpecification.bindings[0].binding = 0;
			skyboxSpecification.bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			skyboxSpecification.bindings[0].descriptorCount = 1;
			skyboxSpecification.bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			skyboxSpecification.bindings[0].pImmutableSamplers = nullptr;

			// cubemap
			skyboxSpecification.bindings[1].binding = 1;
			skyboxSpecification.bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			skyboxSpecification.bindings[1].descriptorCount = 1;
			skyboxSpecification.bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			skyboxSpecification.bindings[1].pImmutableSamplers = nullptr;

			// create
			mPipelines.Insert("Skybox", CreateShared<Vulkan::Pipeline>(mDevice, skyboxSpecification, s_PipelineCache));
			mPipelines.GetRef("Skybox")->GetCreateInfoRef().RSCI.cullMode = VK_CULL_MODE_FRONT_BIT;
			mPipelines.GetRef("Skybox")->Build();
		}

		// grid
		{
			// remove previously 
			if (mPipelines.Exists("Grid")) {
				mPipelines.Erase("Grid");
			}

			Renderer::Vulkan::Pipeline::CreateInfo gridSpecificaiton = {};
			gridSpecificaiton.renderPass = mMainRenderpass;
			gridSpecificaiton.vertexShader = CreateShared<Vulkan::Shader>(mDevice, Vulkan::ShaderType::Vertex, "Grid.vert", GetAssetSubDir("Shader/grid.vert").c_str());
			gridSpecificaiton.fragmentShader = CreateShared<Vulkan::Shader>(mDevice, Vulkan::ShaderType::Fragment, "Grid.frag", GetAssetSubDir("Shader/grid.frag").c_str());
			gridSpecificaiton.vertexComponents = { };
			gridSpecificaiton.passingVertexData = false;
			gridSpecificaiton.bindings.resize(1);

			// camera ubo
			gridSpecificaiton.bindings[0].binding = 0;
			gridSpecificaiton.bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			gridSpecificaiton.bindings[0].descriptorCount = 1;
			gridSpecificaiton.bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
			gridSpecificaiton.bindings[0].pImmutableSamplers = nullptr;

			// create
			mPipelines.Insert("Grid", CreateShared<Renderer::Vulkan::Pipeline>(mDevice, gridSpecificaiton, s_PipelineCache));
			mPipelines.GetRef("Grid")->Build();
		}
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

			// not rendering on the swapchain render pass, as we'are using the viewport to draw stuff into

			vkCmdEndRenderPass(cmdBuffer);

			// end command buffer
			COSMOS_ASSERT(vkEndCommandBuffer(cmdBuffer) == VK_SUCCESS, "Failed to end command buffer recording");
		}

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
			
			// render ui 
			GUI::GetRef().OnRender();

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
			GUI::GetRef().DrawBackendData(cmdBuffer);

			vkCmdEndRenderPass(cmdBuffer);

			COSMOS_ASSERT(vkEndCommandBuffer(cmdBuffer) == VK_SUCCESS, "Failed to end command buffer recording");
		}
	}
}
#endif