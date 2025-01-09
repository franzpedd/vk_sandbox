#include "Viewport.h"

#include "Gizmos.h"
#include "Grid.h"
#include "Core/Application.h"
#include "Widget/PrefabHierarchy.h"
#include <Common/Core/Defines.h>
#include <Common/Debug/Logger.h>
#include <Engine/Core/Project.h>
#include <Engine/Core/Scene.h>
#include <Engine/Entity/Camera.h>
#include <Platform/Core/MainWindow.h>
#include <Platform/Event/WindowEvent.h>
#include <Platform/Event/MouseEvent.h>
#include <Renderer/Core/IContext.h>
#include <Renderer/Core/IGUI.h>
#include <Renderer/GUI/Icon.h>
#include <Renderer/Vulkan/Context.h>
#include <Renderer/Vulkan/Device.h>
#include <Renderer/Vulkan/Pipeline.h>
#include <Renderer/Vulkan/Renderpass.h>
#include <Renderer/Vulkan/Swapchain.h>
#include <Renderer/Vulkan/Picking.h>

namespace Cosmos::Editor
{
	Viewport::Viewport(Application* application, PrefabHierarchy* prefabHierarchy)
		: Widget("Viewport"), mApplication(application), mPrefabHierarchy(prefabHierarchy)
	{
		mGizmos = CreateUnique<Gizmos>();
		mGizmos->SetSnapping(mApplication->GetProjectRef()->GetSettingsRef().gizmo_snapping);
		mGizmos->SetSnappingValue(mApplication->GetProjectRef()->GetSettingsRef().gizmo_snapping_value);
		
		mGrid = CreateUnique<Grid>();
		mGrid->SetVisibility(mApplication->GetProjectRef()->GetSettingsRef().grid_visible);
		
		CreateRendererResources();
	}

	Viewport::~Viewport()
	{
		Renderer::Vulkan::Context* renderer = (Renderer::Vulkan::Context*)(Renderer::IContext::GetRef());
		auto device = renderer->GetDevice();
		auto swapchain = renderer->GetSwapchain();

		vkDeviceWaitIdle(device->GetLogicalDevice());
		vkDestroySampler(device->GetLogicalDevice(), mSampler, nullptr);
		vkDestroyImageView(device->GetLogicalDevice(), mDepthView, nullptr);
		vmaDestroyImage(device->GetAllocator(), mDepthImage, mDepthMemory);
		
		vkDestroyImageView(device->GetLogicalDevice(), mColorView, nullptr);
		vmaDestroyImage(device->GetAllocator(), mColorImage, mColorMemory);
	}

	void Viewport::OnUpdate()
	{
		ImGui::Begin(ICON_FA_CAMERA " Viewport", nullptr);

		auto& boundaries = Renderer::IContext::GetRef()->GetViewportBoundariesRef();
		boundaries.position = { ImGui::GetWindowPos().x,ImGui::GetWindowPos().y };

		ImGui::Image(mDescriptorSet, ImGui::GetContentRegionAvail());
		
		// updating aspect ratio for the docking
		boundaries.size = { ImGui::GetWindowWidth(), ImGui::GetWindowHeight() };
		Engine::Camera::GetRef().SetAspectRatio(boundaries.size.x / boundaries.size.y);
		
		// viewport boundaries
		boundaries.min = { ImGui::GetWindowContentRegionMin().x, ImGui::GetWindowContentRegionMin().y };
		boundaries.min.x += ImGui::GetWindowPos().x;
		boundaries.min.y += ImGui::GetWindowPos().y;

		boundaries.max = { ImGui::GetWindowContentRegionMax().x, ImGui::GetWindowContentRegionMax().y };
		boundaries.max.x += ImGui::GetWindowPos().x;
		boundaries.max.y += ImGui::GetWindowPos().y;
		
		DrawMenu();

		mGizmos->OnUpdate(mPrefabHierarchy->GetSelectedEntity(), boundaries.size);

		ImGui::End();
	}

	void Viewport::OnRender()
	{
		mGrid->OnRender();
	}

	void Viewport::OnEvent(Shared<Platform::EventBase> event)
	{
		mGrid->OnEvent(event);

		if (event->GetType() == Platform::EventType::WindowResize)
		{
			Renderer::Vulkan::Context* renderer = (Renderer::Vulkan::Context*)(Renderer::IContext::GetRef());
			auto device = renderer->GetDevice();
			auto swapchain = renderer->GetSwapchain();
			auto& framebuffers = renderer->GetRenderpassesLibraryRef().GetRef("Viewport")->GetFramebuffersRef();

			vkDestroyImageView(device->GetLogicalDevice(), mDepthView, nullptr);
			vmaDestroyImage(device->GetAllocator(), mDepthImage, mDepthMemory);

			vkDestroyImageView(device->GetLogicalDevice(), mColorView, nullptr);
			vmaDestroyImage(device->GetAllocator(), mColorImage, mColorMemory);

			for (auto& framebuffer : framebuffers) {
				vkDestroyFramebuffer(device->GetLogicalDevice(), framebuffer, nullptr);
			}

			CreateFramebufferResources();
		}

		if (event->GetType() == Platform::EventType::MousePress) {
			auto& boundaries = Renderer::IContext::GetRef()->GetViewportBoundariesRef();
			glm::vec2 framebufferSize = Platform::MainWindow::GetRef().GetFrameBufferSize();
			glm::vec2 mousePos = Platform::MainWindow::GetRef().GetCursorPos();

			if (mousePos.x <= 0 || mousePos.y <= 0) {
				return;
			}
			
			bool clickedInViewport = true;
			clickedInViewport &= mousePos.x >= boundaries.min.x && mousePos.x <= boundaries.max.x;
			clickedInViewport &= mousePos.y >= boundaries.min.y && mousePos.y <= boundaries.max.y;

			if (clickedInViewport) {
				Renderer::Vulkan::Context* renderer = (Renderer::Vulkan::Context*)Renderer::IContext::GetRef();
				renderer->GetPickingRef()->ReadImagePixels(Platform::MainWindow::GetRef().GetViewportCursorPos(boundaries.position, boundaries.size));
			}
		}
	}

	void Viewport::DrawMenu()
	{
		auto& boundaries = Renderer::IContext::GetRef()->GetViewportBoundariesRef();
		ImGui::SetNextWindowPos({ boundaries.position.x + 10.0f, boundaries.position.y + 30.0f });

		ImGui::BeginChild("##ViewportMenubar", ImVec2(), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoBackground);
		
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));

		// gizmos mode
		{
			static uint32_t selectedGizmos = 0;
			GizmosMode modes[4] = { GizmosMode::Undefined, GizmosMode::Translate, GizmosMode::Rotate, GizmosMode::Scale };
			std::string texts[4] = { ICON_LC_MOUSE_POINTER, ICON_LC_MOVE_3D, ICON_LC_ROTATE_3D, ICON_LC_SCALE_3D };
			std::string tooltips[4] = { "Selection", "Translation", "Rotation", "Scale" };
			
			for (uint8_t i = 0; i < 4; i++) {
			
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5.0f);
			
				bool coloredButton = selectedGizmos == i;
				if (coloredButton) {
					ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
				}
			
				if (ImGui::Button(texts[i].c_str())) {
					mGizmos->SetMode(modes[i]);
					selectedGizmos = i;
				}
			
				if (coloredButton) {
					ImGui::PopStyleColor();
				}
			
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
					ImGui::SetTooltip("%s", tooltips[i].c_str());
				}
			
				ImGui::SameLine();
			}
		}

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5.0f);
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
		ImGui::SameLine();

		// grid
		{
			bool selectedButton = false;

			//
			float snapping = mGizmos->GetSnappingValue();
			ImGui::PushItemWidth(50);
			ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 2.0f);
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5.0f);
			if (ImGui::SliderFloat("##Snapping", &snapping, 0.005f, 10.0f, "%.2f")) {
				mGizmos->SetSnappingValue(snapping);
			}
			ImGui::PopStyleVar();
			ImGui::PopItemWidth();

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
				ImGui::SetTooltip("Grid snapping value");
			}

			ImGui::SameLine();

			//
			bool selectedSnapping = mGizmos->GetSnapping();
			selectedButton = selectedSnapping;
			if (selectedButton) {
				ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
			}

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5.0f);
			if (ImGui::Button(ICON_LC_MAGNET)) {
				mGizmos->SetSnapping(!mGizmos->GetSnapping());
			}

			if (selectedButton) {
				ImGui::PopStyleColor();
			}

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
				ImGui::SetTooltip("Enables/Disables snapping with the grid value");
			}

			ImGui::SameLine();

			//
			static bool selectedGrid = true;
			selectedButton = selectedGrid;
			if (selectedButton) {
				ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
			}

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5.0f);
			if (ImGui::Button(ICON_LC_GRID_3X3)) {
				mGrid->SetVisibility(!mGrid->GetVisibility());
				selectedGrid = !selectedGrid;
			}

			if (selectedButton) {
				ImGui::PopStyleColor();
			}
			
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
				ImGui::SetTooltip("Enables/Disables grid on 3D View");
			}
		}
		
		ImGui::PopStyleColor();

		ImGui::EndChild();
	}

	void Viewport::CreateRendererResources()
	{
		// sets the main renderpass to the viewport
		Renderer::Vulkan::Context* renderer = (Renderer::Vulkan::Context*)(Renderer::IContext::GetRef());
		auto device = renderer->GetDevice();

		Shared<Renderer::Vulkan::Renderpass> renderpass = CreateShared<Renderer::Vulkan::Renderpass>(device, "Viewport", VK_SAMPLE_COUNT_1_BIT);
		renderer->GetRenderpassesLibraryRef().Insert("Viewport", renderpass);
		renderer->GetMainRenderpassRef() = renderer->GetRenderpassesLibraryRef().GetRef("Viewport");

		// viewport format
		mSurfaceFormat = VK_FORMAT_R8G8B8A8_SRGB;
		mDepthFormat = device->FindSuitableDepthFormat();

		// create render pass
		{
			std::vector<VkAttachmentDescription> attachments = {};
			attachments.resize(2);

			// color attachment
			attachments[0].format = mSurfaceFormat;
			attachments[0].samples = renderpass->GetMSAA();
			attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			// depth attachment
			attachments[1].format = mDepthFormat;
			attachments[1].samples = renderpass->GetMSAA();
			attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentReference colorReference = {};
			colorReference.attachment = 0;
			colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference depthReference = {};
			depthReference.attachment = 1;
			depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpassDescription = {};
			subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDescription.colorAttachmentCount = 1;
			subpassDescription.pColorAttachments = &colorReference;
			subpassDescription.pDepthStencilAttachment = &depthReference;
			subpassDescription.inputAttachmentCount = 0;
			subpassDescription.pInputAttachments = nullptr;
			subpassDescription.preserveAttachmentCount = 0;
			subpassDescription.pPreserveAttachments = nullptr;
			subpassDescription.pResolveAttachments = nullptr;

			// subpass dependencies for layout transitions
			std::vector<VkSubpassDependency> dependencies = {};
			dependencies.resize(2);

			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			dependencies[0].dependencyFlags = 0;

			dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].dstSubpass = 0;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[1].srcAccessMask = 0;
			dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			dependencies[1].dependencyFlags = 0;

			VkRenderPassCreateInfo renderPassCI = {};
			renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassCI.attachmentCount = (uint32_t)attachments.size();
			renderPassCI.pAttachments = attachments.data();
			renderPassCI.subpassCount = 1;
			renderPassCI.pSubpasses = &subpassDescription;
			renderPassCI.dependencyCount = (uint32_t)dependencies.size();
			renderPassCI.pDependencies = dependencies.data();
			COSMOS_ASSERT(vkCreateRenderPass(device->GetLogicalDevice(), &renderPassCI, nullptr, &renderpass->GetRenderpassRef()) == VK_SUCCESS, "Failed to create renderpass");
		}

		// command pool
		{
			Renderer::Vulkan::Device::QueueFamilyIndices indices = device->FindQueueFamilies(device->GetPhysicalDevice(), device->GetSurface());

			VkCommandPoolCreateInfo cmdPoolInfo = {};
			cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			cmdPoolInfo.queueFamilyIndex = indices.graphics.value();
			cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			COSMOS_ASSERT(vkCreateCommandPool(device->GetLogicalDevice(), &cmdPoolInfo, nullptr, &renderpass->GetCommandPoolRef()) == VK_SUCCESS, "Failed to create command pool");
		}

		// command buffers
		{
			renderpass->GetCommandfuffersRef().resize(CONCURENTLY_RENDERED_FRAMES);

			VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
			cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmdBufferAllocInfo.commandPool = renderpass->GetCommandPoolRef();
			cmdBufferAllocInfo.commandBufferCount = (uint32_t)renderpass->GetCommandfuffersRef().size();
			COSMOS_ASSERT(vkAllocateCommandBuffers(device->GetLogicalDevice(), &cmdBufferAllocInfo, renderpass->GetCommandfuffersRef().data()) == VK_SUCCESS, "Failed to allocate command buffers");
		}

		// sampler
		{
			VkSamplerCreateInfo samplerCI = {};
			samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerCI.magFilter = VK_FILTER_LINEAR;
			samplerCI.minFilter = VK_FILTER_LINEAR;
			samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerCI.anisotropyEnable = VK_TRUE;
			samplerCI.maxAnisotropy = device->GetPropertiesRef().limits.maxSamplerAnisotropy;
			samplerCI.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			samplerCI.unnormalizedCoordinates = VK_FALSE;
			samplerCI.compareEnable = VK_FALSE;
			samplerCI.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			COSMOS_ASSERT(vkCreateSampler(device->GetLogicalDevice(), &samplerCI, nullptr, &mSampler) == VK_SUCCESS, "Failed to create sampler");
		}

		// framebuffers are directly tied to the renderpass, therefore we must create all it's resources
		CreateFramebufferResources();

		// recreate pipelines to match new renderpass
		Renderer::Vulkan::DefaultPipelinesCreateInfo ci = { renderer->GetDevice(), renderer->GetMainRenderpassRef(), renderer->GetPipelinesLibraryRef(), renderer->GetRenderpassesLibraryRef() };
		Renderer::Vulkan::CreateDefaultPipelines(ci);
	}

	void Viewport::CreateFramebufferResources()
	{
		Renderer::Vulkan::Context* renderer = (Renderer::Vulkan::Context*)(Renderer::IContext::GetRef());
		auto device = renderer->GetDevice();
		auto swapchain = renderer->GetSwapchain();
		auto& renderpass = renderer->GetRenderpassesLibraryRef().GetRef("Viewport");
		size_t imageCount = swapchain->GetImagesRef().size();

		// depth buffer
		{
			device->CreateImage
			(
				swapchain->GetExtent().width,
				swapchain->GetExtent().height,
				1,
				1,
				renderpass->GetMSAA(),
				mDepthFormat,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				mDepthImage,
				mDepthMemory
			);

			mDepthView = device->CreateImageView
			(
				mDepthImage,
				mDepthFormat,
				VK_IMAGE_ASPECT_DEPTH_BIT
			);
		}

		// color image
		device->CreateImage
		(
			swapchain->GetExtent().width,
			swapchain->GetExtent().height,
			1,
			1,
			renderpass->GetMSAA(),
			mSurfaceFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, // for picking
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			mColorImage,
			mColorMemory
		);

		mColorView = device->CreateImageView
		(
			mColorImage,
			mSurfaceFormat,
			VK_IMAGE_ASPECT_COLOR_BIT
		);

		VkCommandBuffer command = device->BeginSingleTimeCommand(renderpass->GetCommandPoolRef());

		device->InsertImageMemoryBarrier
		(
			command,
			mColorImage,
			VK_ACCESS_TRANSFER_READ_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED, // must get from last render pass (undefined also works)
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, // must set for next render pass
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
		);

		device->EndSingleTimeCommand(renderpass->GetCommandPoolRef(), command);

		mDescriptorSet = (VkDescriptorSet)Renderer::IGUI::GetRef()->AddTexture(mSampler, mColorView);

		// frame buffers
		{
			renderpass->GetFramebuffersRef().resize(imageCount);

			for (size_t i = 0; i < imageCount; i++)
			{
				std::vector<VkImageView> attachments = { mColorView, mDepthView };
				attachments.resize(2);

				VkFramebufferCreateInfo framebufferCI = {};
				framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferCI.renderPass = renderpass->GetRenderpassRef();
				framebufferCI.attachmentCount = (uint32_t)attachments.size();
				framebufferCI.pAttachments = attachments.data();
				framebufferCI.width = swapchain->GetExtent().width;
				framebufferCI.height = swapchain->GetExtent().height;
				framebufferCI.layers = 1;
				COSMOS_ASSERT(vkCreateFramebuffer(device->GetLogicalDevice(), &framebufferCI, nullptr, &renderpass->GetFramebuffersRef()[i]) == VK_SUCCESS, "Failed to create framebuffer");
			}
		}
	}
}