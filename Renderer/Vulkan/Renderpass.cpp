#if defined RENDERER_VULKAN
#include "Renderpass.h"

#include "Device.h"

namespace Cosmos::Renderer::Vulkan
{
	Renderpass::Renderpass(Shared<Device> device, const char* name, VkSampleCountFlagBits msaa)
		: mDevice(device), mName(name), mMSAA(msaa)
	{
	}

	Renderpass::~Renderpass()
	{
		vkDeviceWaitIdle(mDevice->GetLogicalDevice());

		vkDestroyDescriptorPool(mDevice->GetLogicalDevice(), mDescriptorPool, nullptr);
		vkDestroyRenderPass(mDevice->GetLogicalDevice(), mRenderPass, nullptr);

		vkFreeCommandBuffers(mDevice->GetLogicalDevice(), mCommandPool, (uint32_t)mCommandfuffers.size(), mCommandfuffers.data());
		vkDestroyCommandPool(mDevice->GetLogicalDevice(), mCommandPool, nullptr);

		for (auto& framebuffer : mFramebuffers) {
			vkDestroyFramebuffer(mDevice->GetLogicalDevice(), framebuffer, nullptr);
		}

		mFramebuffers.clear();
	}
}
#endif