#include "Picking.h"

#include "Device.h"
#include "Renderpass.h"
#include "Swapchain.h"
#include "Core/Defines.h"
#include "Core/IContext.h"

#include <Common/Debug/Logger.h>
#include <Common/Math/Math.h>
#include <Engine/Core/Application.h>
#include <Platform/Core/MainWindow.h>
#include <Platform/Event/WindowEvent.h>

namespace Cosmos::Renderer::Vulkan
{
	Picking::Picking(Engine::Application* application, Shared<Device> device, Shared<Swapchain> swapchain, Library<Shared<Renderpass>>& renderpassesLib)
		: mApplication(application), mDevice(device), mSwapchain(swapchain), mRenderpassesLib(renderpassesLib)
	{
		CreateRenderpass();
		CreateImages();
	}

	Picking::~Picking()
	{
		vkDeviceWaitIdle(mDevice->GetLogicalDevice());
		vkDestroyImageView(mDevice->GetLogicalDevice(), mDepthView, nullptr);
		vmaDestroyImage(mDevice->GetAllocator(), mDepthImage, mDepthMemory);

		for (size_t i = 0; i < mSwapchain->GetImagesRef().size(); i++) {
			vkDestroyImageView(mDevice->GetLogicalDevice(), mColorViews[i], nullptr);
			vmaDestroyImage(mDevice->GetAllocator(), mColorImages[i], mColorMemories[i]);
		}
	}

	void Picking::OnEvent(Shared<Platform::EventBase> event)
	{
		if (event->GetType() == Platform::EventType::WindowResize) {
			auto& framebuffers = mRenderpassesLib.GetRef("Picking")->GetFramebuffersRef();

			vkDestroyImageView(mDevice->GetLogicalDevice(), mDepthView, nullptr);
			vmaDestroyImage(mDevice->GetAllocator(), mDepthImage, mDepthMemory);

			for (size_t i = 0; i < mSwapchain->GetImagesRef().size(); i++) {
				vkDestroyImageView(mDevice->GetLogicalDevice(), mColorViews[i], nullptr);
				vmaDestroyImage(mDevice->GetAllocator(), mColorImages[i], mColorMemories[i]);
			}

			for (auto& framebuffer : framebuffers) {
				vkDestroyFramebuffer(mDevice->GetLogicalDevice(), framebuffer, nullptr);
			}

			CreateImages();
		}
	}

	void Picking::ManageRenderpass(uint32_t currentFrame, uint32_t swapchainIndex)
	{
		std::vector<VkClearValue> clearValues(2);
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 0.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		// swapchain render pass is guaranteed to exists
		{
			VkCommandBuffer& cmdBuffer = mRenderpassesLib.GetRef("Picking")->GetCommandfuffersRef()[currentFrame];
			VkFramebuffer& frameBuffer = mRenderpassesLib.GetRef("Picking")->GetFramebuffersRef()[swapchainIndex];
			VkRenderPass& renderPass = mRenderpassesLib.GetRef("Picking")->GetRenderpassRef();

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
			VkExtent2D extent = mSwapchain->GetExtent();
			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)extent.width;
			viewport.height = (float)extent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

			auto& boundaries = IContext::GetRef()->GetViewportBoundariesRef();
			glm::vec2 mousePos = glm::clamp(Platform::MainWindow::GetRef().GetViewportCursorPos(boundaries.position, boundaries.size), glm::vec2(0.0f, 0.0f), glm::vec2(extent.width, extent.height));
			
			VkRect2D scissor = {};
			scissor.offset = { (int32_t)mousePos.x, (int32_t)mousePos.y };
			scissor.extent = { 1, 1 };
			vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

			// render objects
			mApplication->OnRender(IContext::Stage::Picking);

			// end render pass
			vkCmdEndRenderPass(cmdBuffer);

			// end command buffer
			COSMOS_ASSERT(vkEndCommandBuffer(cmdBuffer) == VK_SUCCESS, "Failed to end command buffer recording");
		}
	}

	void Picking::ReadImagePixels(glm::vec2 pos)
	{
		auto renderpass = mRenderpassesLib.GetRef("Picking");
		VkCommandBuffer cmdBuffer = mDevice->BeginSingleTimeCommand(renderpass->GetCommandPoolRef());

		// convert the image into transferable
		mDevice->InsertImageMemoryBarrier
		(
			cmdBuffer,
			mColorImages[mSwapchain->GetImageIndexRef()], // we'll be testing with only one image for now
			VK_ACCESS_TRANSFER_READ_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
		);

		// create the staging buffer
		VkBuffer stagingBuffer;
		VmaAllocation stagingBufferMemory;

		mDevice->CreateBuffer
		(
			VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			mImageSize,
			&stagingBuffer,
			&stagingBufferMemory
		);

		// copy the image to the buffer
		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { (int32_t)pos.x, (int32_t)pos.y, 0 };
		region.imageExtent = { 1, 1, 1 };
		vkCmdCopyImageToBuffer(cmdBuffer, mColorImages[mSwapchain->GetImageIndexRef()], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer, 1, &region);

		// convert the image into shader optimal again
		mDevice->InsertImageMemoryBarrier
		(
			cmdBuffer,
			mColorImages[mSwapchain->GetImageIndexRef()], // we'll be testing with only one image for now
			VK_ACCESS_TRANSFER_READ_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
		);

		mDevice->EndSingleTimeCommand(renderpass->GetCommandPoolRef(), cmdBuffer);

		// map the buffer memory and read the pixels
		void* data;
		vmaMapMemory(mDevice->GetAllocator(), stagingBufferMemory, &data);

		uint32_t* mappedData = reinterpret_cast<uint32_t*>(data);
		uint32_t x = mappedData[0]; // first component of vec2
		uint32_t y = mappedData[1]; // second component of vec2
		uint32_t lowerBits = (uint32_t)x; // first float as uint
		uint32_t upperBits = (uint32_t)y; // second float as uint
		uint64_t reconstructedValue = ((uint64_t)upperBits << 32) | lowerBits;
		COSMOS_LOG(Logger::Trace, "ID: %llu", reconstructedValue);

		vmaUnmapMemory(mDevice->GetAllocator(), stagingBufferMemory);

		// cleanup
		vmaDestroyBuffer(mDevice->GetAllocator(), stagingBuffer, stagingBufferMemory);
	}

	void Picking::CreateRenderpass()
	{
		Shared<Renderer::Vulkan::Renderpass> renderpass = CreateShared<Renderer::Vulkan::Renderpass>(mDevice, "Picking", VK_SAMPLE_COUNT_1_BIT);
		mRenderpassesLib.Insert("Picking", renderpass);

		mSurfaceFormat = VK_FORMAT_R32G32_UINT;
		mImageSize = 2 * 8; // (RED + GREEN) * 8 bits
		mDepthFormat = mDevice->FindSuitableDepthFormat();

		// create render pass
		{
			std::vector<VkAttachmentDescription> attachments = {};
			attachments.resize(2);

			// color attachment
			attachments[0].format = mSurfaceFormat;
			attachments[0].samples = mMSAA;
			attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			// depth attachment
			attachments[1].format = mDepthFormat;
			attachments[1].samples = mMSAA;
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
			COSMOS_ASSERT(vkCreateRenderPass(mDevice->GetLogicalDevice(), &renderPassCI, nullptr, &renderpass->GetRenderpassRef()) == VK_SUCCESS, "Failed to create renderpass");
		}

		// command pool
		{
			Renderer::Vulkan::Device::QueueFamilyIndices indices = mDevice->FindQueueFamilies(mDevice->GetPhysicalDevice(), mDevice->GetSurface());

			VkCommandPoolCreateInfo cmdPoolInfo = {};
			cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			cmdPoolInfo.queueFamilyIndex = indices.graphics.value();
			cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			COSMOS_ASSERT(vkCreateCommandPool(mDevice->GetLogicalDevice(), &cmdPoolInfo, nullptr, &renderpass->GetCommandPoolRef()) == VK_SUCCESS, "Failed to create command pool");
		}

		// command buffers
		{
			renderpass->GetCommandfuffersRef().resize(CONCURENTLY_RENDERED_FRAMES);

			VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
			cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmdBufferAllocInfo.commandPool = renderpass->GetCommandPoolRef();
			cmdBufferAllocInfo.commandBufferCount = (uint32_t)renderpass->GetCommandfuffersRef().size();
			COSMOS_ASSERT(vkAllocateCommandBuffers(mDevice->GetLogicalDevice(), &cmdBufferAllocInfo, renderpass->GetCommandfuffersRef().data()) == VK_SUCCESS, "Failed to allocate command buffers");
		}
	}

	void Picking::CreateImages()
	{
		VkExtent2D size = mSwapchain->GetExtent();

		// depth buffer
		{
			mDevice->CreateImage
			(
				size.width,
				size.height,
				1,
				1,
				mMSAA,
				mDepthFormat,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				mDepthImage,
				mDepthMemory
			);

			mDepthView = mDevice->CreateImageView
			(
				mDepthImage,
				mDepthFormat,
				VK_IMAGE_ASPECT_DEPTH_BIT
			);
		}

		// color images
		{
			size_t imageCount = mSwapchain->GetImagesRef().size();
			auto renderpass = mRenderpassesLib.GetRef("Picking");
			renderpass->GetFramebuffersRef().resize(imageCount);
			mColorImages.resize(imageCount);
			mColorMemories.resize(imageCount);
			mColorViews.resize(imageCount);

			for (size_t i = 0; i < imageCount; i++) {
				mDevice->CreateImage
				(
					size.width,
					size.height,
					1,
					1,
					mMSAA,
					mSurfaceFormat,
					VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, // for picking
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					mColorImages[i],
					mColorMemories[i]
				);

				VkCommandBuffer command = mDevice->BeginSingleTimeCommand(renderpass->GetCommandPoolRef());

				mDevice->InsertImageMemoryBarrier
				(
					command,
					mColorImages[i],
					VK_ACCESS_TRANSFER_READ_BIT,
					VK_ACCESS_MEMORY_READ_BIT,
					VK_IMAGE_LAYOUT_UNDEFINED, // must get from last render pass (undefined also works)
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, // must set for next render pass
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
				);

				mDevice->EndSingleTimeCommand(renderpass->GetCommandPoolRef(), command);

				mColorViews[i] = mDevice->CreateImageView(mColorImages[i], mSurfaceFormat, VK_IMAGE_ASPECT_COLOR_BIT);

				std::vector<VkImageView> attachments = { mColorViews[i], mDepthView };
				attachments.resize(2);

				VkFramebufferCreateInfo framebufferCI = {};
				framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferCI.renderPass = renderpass->GetRenderpassRef();
				framebufferCI.attachmentCount = (uint32_t)attachments.size();
				framebufferCI.pAttachments = attachments.data();
				framebufferCI.width = mSwapchain->GetExtent().width;
				framebufferCI.height = mSwapchain->GetExtent().height;
				framebufferCI.layers = 1;
				COSMOS_ASSERT(vkCreateFramebuffer(mDevice->GetLogicalDevice(), &framebufferCI, nullptr, &renderpass->GetFramebuffersRef()[i]) == VK_SUCCESS, "Failed to create framebuffer");
			}
		}
	}
}