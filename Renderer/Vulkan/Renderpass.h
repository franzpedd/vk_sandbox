#pragma once
#if defined RENDERER_VULKAN
#include "Wrapper/vulkan.h"

#include <Common/Util/Memory.h>
#include <vector>

// forward declaration
namespace Cosmos::Renderer::Vulkan { class Device; }

namespace Cosmos::Renderer::Vulkan
{
	class Renderpass
	{
	public:

		// constructor
		Renderpass(Shared<Device> device, const char* name, VkSampleCountFlagBits msaa);

		// destructor
		~Renderpass();

		// returns the used msaa for the renderpass
		inline VkSampleCountFlagBits GetMSAA() const { return mMSAA; }

		// returns the ptr to the renderpass
		inline VkRenderPass& GetRenderpassRef() { return mRenderPass; }

		// returns the ptr to the command pool
		inline VkCommandPool& GetCommandPoolRef() { return mCommandPool; }

		// returns the ptr to the descriptor pool
		inline VkDescriptorPool& GetDescriptorPoolRef() { return mDescriptorPool; }

		// returns a reference to the vector of command buffers
		inline std::vector<VkCommandBuffer>& GetCommandfuffersRef() { return mCommandfuffers; }

		// returns a referencec to the vector of frame buffers
		inline std::vector<VkFramebuffer>& GetFramebuffersRef() { return mFramebuffers; }

	private:

		Shared<Device> mDevice;
		const char* mName = nullptr;
		VkSampleCountFlagBits mMSAA = VK_SAMPLE_COUNT_1_BIT;

		VkRenderPass mRenderPass = VK_NULL_HANDLE;
		VkCommandPool mCommandPool = VK_NULL_HANDLE;
		VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;

		std::vector<VkCommandBuffer> mCommandfuffers = {};
		std::vector<VkFramebuffer> mFramebuffers = {};
	};
}
#endif