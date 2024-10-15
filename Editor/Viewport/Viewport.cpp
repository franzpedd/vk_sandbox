#include "Viewport.h"

#include "Gizmos.h"
#include "Grid.h"
#include <Common/Debug/Logger.h>
#include <Engine/Entity/Camera.h>
#include <Platform/Event/WindowEvent.h>
#include <Renderer/Core/Context.h>
#include <Renderer/Core/Defines.h>
#include <Renderer/GUI/GUI.h>
#include <Renderer/GUI/Icon.h>
#include <Renderer/Vulkan/Device.h>
#include <Renderer/Vulkan/Renderpass.h>
#include <Renderer/Vulkan/Swapchain.h>

namespace Cosmos::Editor
{
	Viewport::Viewport()
		: Widget("Viewport")
	{
		mGizmos = CreateUnique<Gizmos>();
		mGrid = CreateUnique<Grid>();
		CreateRendererResources();
	}

	Viewport::~Viewport()
	{
		auto device = Renderer::Context::GetRef().GetDevice();
		auto swapchain = Renderer::Context::GetRef().GetSwapchain();

		vkDeviceWaitIdle(device->GetLogicalDevice());
		vkDestroySampler(device->GetLogicalDevice(), mSampler, nullptr);
		vkDestroyImageView(device->GetLogicalDevice(), mDepthView, nullptr);
		vmaDestroyImage(device->GetAllocator(), mDepthImage, mDepthMemory);
		
		for (size_t i = 0; i < swapchain->GetImagesRef().size(); i++) {
			vkDestroyImageView(device->GetLogicalDevice(), mImageViews[i], nullptr);
			vmaDestroyImage(device->GetAllocator(), mImages[i], mImageMemories[i]);
		}
	}

	void Viewport::OnUpdate()
	{
		if (ImGui::Begin(ICON_FA_CAMERA " Viewport", nullptr /*, ImGuiWindowFlags_MenuBar*/))
		{
			ImGui::Image(mDescriptorSets[Renderer::Context::GetRef().GetCurrentFrame()], ImGui::GetContentRegionAvail());
		
			// updating aspect ratio for the docking
			mCurrentSize = ImGui::GetWindowSize();
			Engine::Camera::GetRef().SetAspectRatio(mCurrentSize.x / mCurrentSize.y);
		
			// viewport boundaries
			mContentRegionMin = ImGui::GetWindowContentRegionMin();
			mContentRegionMax = ImGui::GetWindowContentRegionMax();
			mContentRegionMin.x += ImGui::GetWindowPos().x;
			mContentRegionMin.y += ImGui::GetWindowPos().y;
			mContentRegionMax.x += ImGui::GetWindowPos().x;
			mContentRegionMax.y += ImGui::GetWindowPos().y;
		
			//DrawMenuBar();
		
			//mGizmos->OnUpdate(mSceneHierarchy->GetSelectedEntityRef(), mCurrentSize);
		}
		
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
			auto device = Renderer::Context::GetRef().GetDevice();
			auto swapchain = Renderer::Context::GetRef().GetSwapchain();
			auto& framebuffers = Renderer::Context::GetRef().GetRenderpassesLibraryRef().GetRef("Viewport")->GetFramebuffersRef();

			vkDestroyImageView(device->GetLogicalDevice(), mDepthView, nullptr);
			vmaDestroyImage(device->GetAllocator(), mDepthImage, mDepthMemory);

			for (size_t i = 0; i < swapchain->GetImagesRef().size(); i++) {
				vkDestroyImageView(device->GetLogicalDevice(), mImageViews[i], nullptr);
				vmaDestroyImage(device->GetAllocator(), mImages[i], mImageMemories[i]);
			}

			for (auto& framebuffer : framebuffers) {
				vkDestroyFramebuffer(device->GetLogicalDevice(), framebuffer, nullptr);
			}

			CreateFramebufferResources();
		}
	}

	void Viewport::DrawMenuBar()
	{
		if (ImGui::BeginMenuBar())
		{
			ImGui::EndMenuBar();
		}
	}

	void Viewport::CreateRendererResources()
	{
		// sets the main renderpass to the viewport
		auto& renderer = Renderer::Context::GetRef();
		auto device = renderer.GetDevice();

		Shared<Renderer::Vulkan::Renderpass> renderpass = CreateShared<Renderer::Vulkan::Renderpass>(device, "Viewport", VK_SAMPLE_COUNT_1_BIT);
		renderer.GetRenderpassesLibraryRef().Insert("Viewport", renderpass);
		renderer.GetMainRenderpassRef() = renderer.GetRenderpassesLibraryRef().GetRef("Viewport");

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
		renderer.RecreatePipelines();
	}

	void Viewport::CreateFramebufferResources()
	{
		auto& renderer = Renderer::Context::GetRef();
		auto device = renderer.GetDevice();
		auto swapchain = renderer.GetSwapchain();
		auto& renderpass = renderer.GetRenderpassesLibraryRef().GetRef("Viewport");
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

		// images
		{
			mImages.resize(imageCount);
			mImageMemories.resize(imageCount);
			mImageViews.resize(imageCount);
			mDescriptorSets.resize(imageCount);
			renderpass->GetFramebuffersRef().resize(imageCount);

			for (size_t i = 0; i < imageCount; i++)
			{
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
					mImages[i],
					mImageMemories[i]
				);

				VkCommandBuffer command = device->BeginSingleTimeCommand(renderpass->GetCommandPoolRef());

				device->InsertImageMemoryBarrier
				(
					command,
					mImages[i],
					VK_ACCESS_TRANSFER_READ_BIT,
					VK_ACCESS_MEMORY_READ_BIT,
					VK_IMAGE_LAYOUT_UNDEFINED, // must get from last render pass (undefined also works)
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, // must set for next render pass
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
				);

				device->EndSingleTimeCommand(renderpass->GetCommandPoolRef(), command);

				mImageViews[i] = device->CreateImageView(mImages[i], mSurfaceFormat, VK_IMAGE_ASPECT_COLOR_BIT);
				mDescriptorSets[i] = (VkDescriptorSet)Renderer::GUI::GetRef().AddTexture(mSampler, mImageViews[i]);

				std::vector<VkImageView> attachments = { mImageViews[i], mDepthView };
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