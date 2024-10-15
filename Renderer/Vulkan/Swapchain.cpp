#if defined RENDERER_VULKAN
#include "Swapchain.h"

#include "Device.h"
#include "Renderpass.h"
#include "Core/Defines.h"
#include <Common/Debug/Logger.h>
#include <Platform/Core/MainWindow.h>

#include <algorithm>
#include <array>
#include <limits>

namespace Cosmos::Renderer::Vulkan
{
	Swapchain::Swapchain(Shared<Device> device, Library<Shared<Renderpass>>& renderpassLib)
		: mDevice(device), mRenderpassLib(renderpassLib)
	{
		mRenderpassLib.Insert("Swapchain", CreateShared<Renderpass>(mDevice, "Swapchain", mDevice->GetMSAA()));

		CreateSwapchain();
		CreateImageViews();

		CreateRenderPass();
		CreateFramebuffers();

		CreateCommandPool();
		CreateCommandBuffers();

		CreateSyncSystem();
	}

	Swapchain::~Swapchain()
	{
		vkDeviceWaitIdle(mDevice->GetLogicalDevice());

		for (size_t i = 0; i < CONCURENTLY_RENDERED_FRAMES; i++)
		{
			vkDestroyFence(mDevice->GetLogicalDevice(), mInFlightFences[i], nullptr);
			vkDestroySemaphore(mDevice->GetLogicalDevice(), mRenderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(mDevice->GetLogicalDevice(), mImageAvailableSemaphores[i], nullptr);
		}

		vkDestroyImageView(mDevice->GetLogicalDevice(), mDepthView, nullptr);
		vmaDestroyImage(mDevice->GetAllocator(), mDepthImage, mDepthMemory);

		vkDestroyImageView(mDevice->GetLogicalDevice(), mColorView, nullptr);
		vmaDestroyImage(mDevice->GetAllocator(), mColorImage, mColorMemory);

		for (auto imageView : mImageViews)
		{
			vkDestroyImageView(mDevice->GetLogicalDevice(), imageView, nullptr);
		}

		mImageViews.clear();

		vkDestroySwapchainKHR(mDevice->GetLogicalDevice(), mSwapchain, nullptr);
	}

	void Swapchain::CreateSwapchain()
	{
		Details details = QueryDetails();

		// query all important details of the swapchain
		mSurfaceFormat = ChooseSurfaceFormat(details.formats);
		mPresentMode = ChoosePresentMode(details.presentModes);
		mExtent = ChooseExtent(details.capabilities);

		mImageCount = details.capabilities.minImageCount + 1;
		if (details.capabilities.maxImageCount > 0 && mImageCount > details.capabilities.maxImageCount)
		{
			mImageCount = details.capabilities.maxImageCount;
		}

		// create the swapchain
		Device::QueueFamilyIndices indices = mDevice->FindQueueFamilies(mDevice->GetPhysicalDevice(), mDevice->GetSurface());
		uint32_t queueFamilyIndices[] = { indices.graphics.value(), indices.present.value() };

		VkSwapchainCreateInfoKHR swapchainCI = {};
		swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCI.pNext = nullptr;
		swapchainCI.flags = 0;
		swapchainCI.surface = mDevice->GetSurface();
		swapchainCI.minImageCount = mImageCount;
		swapchainCI.imageFormat = mSurfaceFormat.format;
		swapchainCI.imageColorSpace = mSurfaceFormat.colorSpace;
		swapchainCI.imageExtent = mExtent;
		swapchainCI.imageArrayLayers = 1;
		swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT; // for copying the images, allowing viewports

		if (indices.graphics != indices.present)
		{
			swapchainCI.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchainCI.queueFamilyIndexCount = 2;
			swapchainCI.pQueueFamilyIndices = queueFamilyIndices;
		}

		else
		{
			swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		swapchainCI.preTransform = details.capabilities.currentTransform;
		swapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchainCI.presentMode = mPresentMode;
		swapchainCI.clipped = VK_TRUE;

		COSMOS_ASSERT(vkCreateSwapchainKHR(mDevice->GetLogicalDevice(), &swapchainCI, nullptr, &mSwapchain) == VK_SUCCESS, "Failed to create the Swapchain");

		// get the images in the swapchain
		vkGetSwapchainImagesKHR(mDevice->GetLogicalDevice(), mSwapchain, &mImageCount, nullptr);
		mImages.resize(mImageCount);
		vkGetSwapchainImagesKHR(mDevice->GetLogicalDevice(), mSwapchain, &mImageCount, mImages.data());
	}

	void Swapchain::CreateImageViews()
	{
		mImageViews.resize(mImages.size());

		for (size_t i = 0; i < mImages.size(); i++)
		{
			mImageViews[i] = mDevice->CreateImageView(mImages[i], mSurfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
		}
	}

	void Swapchain::CreateRenderPass()
	{
		// attachments descriptions
		std::array<VkAttachmentDescription, 3> attachments = {};

		// color
		attachments[0].format = mSurfaceFormat.format;
		attachments[0].samples = mDevice->GetMSAA();
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// depth
		attachments[1].format = mDevice->FindSuitableDepthFormat();
		attachments[1].samples = mDevice->GetMSAA();
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// resolve
		attachments[2].format = mSurfaceFormat.format;
		attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[2].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		// finalLayout should not be VK_IMAGE_LAYOUT_PRESENT_SRC_KHR as ui is a post render pass that will present

		// attachments references
		std::array<VkAttachmentReference, 3> references = {};

		references[0].attachment = 0;
		references[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		references[1].attachment = 1;
		references[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		references[2].attachment = 2;
		references[2].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// subpass
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &references[0];
		subpass.pDepthStencilAttachment = &references[1];
		subpass.pResolveAttachments = &references[2];

		// subpass dependencies for layout transitions
		std::array<VkSubpassDependency, 2> dependencies;

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
		renderPassCI.pSubpasses = &subpass;
		renderPassCI.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassCI.pDependencies = dependencies.data();

		auto& spec = mRenderpassLib.GetRef("Swapchain");
		COSMOS_ASSERT(vkCreateRenderPass(mDevice->GetLogicalDevice(), &renderPassCI, nullptr, &spec->GetRenderpassRef()) == VK_SUCCESS, "Failed to create render pass");
	}

	void Swapchain::CreateFramebuffers()
	{
		auto& spec = mRenderpassLib.GetRef("Swapchain");

		// create frame color resources
		VkFormat format = mSurfaceFormat.format;

		mDevice->CreateImage
		(
			mExtent.width,
			mExtent.height,
			1,
			1,
			spec->GetMSAA(),
			format,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			mColorImage,
			mColorMemory
		);

		mColorView = mDevice->CreateImageView(mColorImage, format, VK_IMAGE_ASPECT_COLOR_BIT);

		// create frame depth resources
		format = mDevice->FindSuitableDepthFormat();

		mDevice->CreateImage
		(
			mExtent.width,
			mExtent.height,
			1,
			1,
			spec->GetMSAA(),
			format,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			mDepthImage,
			mDepthMemory
		);

		mDepthView = mDevice->CreateImageView(mDepthImage, format, VK_IMAGE_ASPECT_DEPTH_BIT);

		// create frame buffers
		spec->GetFramebuffersRef().resize(mImageViews.size());

		for (size_t i = 0; i < mImageViews.size(); i++)
		{
			std::array<VkImageView, 3> attachments = { mColorView, mDepthView, mImageViews[i] };

			VkFramebufferCreateInfo framebufferCI = {};
			framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferCI.renderPass = spec->GetRenderpassRef();
			framebufferCI.attachmentCount = (uint32_t)attachments.size();
			framebufferCI.pAttachments = attachments.data();
			framebufferCI.width = mExtent.width;
			framebufferCI.height = mExtent.height;
			framebufferCI.layers = 1;
			COSMOS_ASSERT(vkCreateFramebuffer(mDevice->GetLogicalDevice(), &framebufferCI, nullptr, &spec->GetFramebuffersRef()[i]) == VK_SUCCESS, "Failed to create framebuffer");
		}
	}

	void Swapchain::Cleanup()
	{
		vkDestroyImageView(mDevice->GetLogicalDevice(), mDepthView, nullptr);
		vmaDestroyImage(mDevice->GetAllocator(), mDepthImage, mDepthMemory);

		vkDestroyImageView(mDevice->GetLogicalDevice(), mColorView, nullptr);
		vmaDestroyImage(mDevice->GetAllocator(), mColorImage, mColorMemory);

		auto& spec = mRenderpassLib.GetRef("Swapchain");
		for (uint32_t i = 0; i < spec->GetFramebuffersRef().size(); i++)
		{
			vkDestroyFramebuffer(mDevice->GetLogicalDevice(), spec->GetFramebuffersRef()[i], nullptr);
		}

		for (auto imageView : mImageViews)
		{
			vkDestroyImageView(mDevice->GetLogicalDevice(), imageView, nullptr);
		}

		vkDestroySwapchainKHR(mDevice->GetLogicalDevice(), mSwapchain, nullptr);
	}

	void Swapchain::Recreate()
	{
		Platform::MainWindow::GetRef().HintResize(false);

		vkDeviceWaitIdle(mDevice->GetLogicalDevice());

		Cleanup();

		CreateSwapchain();
		CreateImageViews();
		CreateFramebuffers();
	}

	Swapchain::Details Swapchain::QueryDetails()
	{
		Details details = {};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mDevice->GetPhysicalDevice(), mDevice->GetSurface(), &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice->GetPhysicalDevice(), mDevice->GetSurface(), &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice->GetPhysicalDevice(), mDevice->GetSurface(), &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->GetPhysicalDevice(), mDevice->GetSurface(), &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->GetPhysicalDevice(), mDevice->GetSurface(), &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	void Swapchain::CreateSyncSystem()
	{
		mImageAvailableSemaphores.resize(CONCURENTLY_RENDERED_FRAMES);
		mRenderFinishedSemaphores.resize(CONCURENTLY_RENDERED_FRAMES);
		mInFlightFences.resize(CONCURENTLY_RENDERED_FRAMES);

		VkSemaphoreCreateInfo semaphoreCI = {};
		semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCI.pNext = nullptr;
		semaphoreCI.flags = 0;

		VkFenceCreateInfo fenceCI = {};
		fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCI.pNext = nullptr;
		fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < CONCURENTLY_RENDERED_FRAMES; i++)
		{
			COSMOS_ASSERT(vkCreateSemaphore(mDevice->GetLogicalDevice(), &semaphoreCI, nullptr, &mImageAvailableSemaphores[i]) == VK_SUCCESS, "Failed to create image available semaphore");
			COSMOS_ASSERT(vkCreateSemaphore(mDevice->GetLogicalDevice(), &semaphoreCI, nullptr, &mRenderFinishedSemaphores[i]) == VK_SUCCESS, "Failed to create render finished semaphore");
			COSMOS_ASSERT(vkCreateFence(mDevice->GetLogicalDevice(), &fenceCI, nullptr, &mInFlightFences[i]) == VK_SUCCESS, "Failed to create in flight fence");
		}
	}

	void Swapchain::CreateCommandPool()
	{
		Device::QueueFamilyIndices indices = mDevice->FindQueueFamilies(mDevice->GetPhysicalDevice(), mDevice->GetSurface());
		auto& spec = mRenderpassLib.GetRef("Swapchain");

		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = indices.graphics.value();
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		COSMOS_ASSERT(vkCreateCommandPool(mDevice->GetLogicalDevice(), &cmdPoolInfo, nullptr, &spec->GetCommandPoolRef()) == VK_SUCCESS, "Failed to create command pool");
	}

	void Swapchain::CreateCommandBuffers()
	{
		auto& spec = mRenderpassLib.GetRef("Swapchain");
		spec->GetCommandfuffersRef().resize(CONCURENTLY_RENDERED_FRAMES);

		VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
		cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufferAllocInfo.commandPool = spec->GetCommandPoolRef();
		cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufferAllocInfo.commandBufferCount = (uint32_t)spec->GetCommandfuffersRef().size();
		COSMOS_ASSERT(vkAllocateCommandBuffers(mDevice->GetLogicalDevice(), &cmdBufferAllocInfo, spec->GetCommandfuffersRef().data()) == VK_SUCCESS, "Failed to create command buffers");
	}

	VkSurfaceFormatKHR Swapchain::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR Swapchain::ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool vsync)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			// triple-buffer
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		// vsync
		if (vsync) return VK_PRESENT_MODE_FIFO_KHR;

		// render as is
		return VK_PRESENT_MODE_IMMEDIATE_KHR;
	}

	VkExtent2D Swapchain::ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}

		int32_t width, height;
		Platform::MainWindow::GetRef().GetFrameBufferSize(&width, &height);

		VkExtent2D actualExtent = { (uint32_t)width, (uint32_t)height };
		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

#endif