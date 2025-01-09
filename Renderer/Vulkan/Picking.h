#pragma once
#ifdef RENDERER_VULKAN

#include "Wrapper/vulkan.h"
#include <Common/Math/Math.h>
#include <Common/Util/Library.h>
#include <Common/Util/Memory.h>
#include <vector>

// forward declarations
namespace Cosmos::Engine { class Application; }
namespace Cosmos::Platform { class EventBase; }
namespace Cosmos::Renderer::Vulkan { class Device; }
namespace Cosmos::Renderer::Vulkan { class Swapchain; }
namespace Cosmos::Renderer::Vulkan { class Renderpass; }

namespace Cosmos::Renderer::Vulkan
{
	class Picking
	{
	public:

		// constructor
		Picking(Engine::Application* application, Shared<Device> device, Shared<Swapchain> swapchain, Library<Shared<Renderpass>>& renderpassesLib);

		// destructor
		~Picking();

	public:

		// called when an event happens
		void OnEvent(Shared<Platform::EventBase> event);

		// called for sending what to draw on the picking-phase
		void ManageRenderpass(uint32_t currentFrame, uint32_t swapchainIndex);

		// reads the image pixels, for picking
		void ReadImagePixels(glm::vec2 pos);

	private:

		// creates the renderpass used for picking phase
		void CreateRenderpass();

		// creates the images used for picking
		void CreateImages();

	private:

		Engine::Application* mApplication;
		Shared<Device> mDevice;
		Shared<Swapchain> mSwapchain;
		Library<Shared<Renderpass>>& mRenderpassesLib;

		VkSampleCountFlagBits mMSAA = VK_SAMPLE_COUNT_1_BIT;
		VkFormat mSurfaceFormat = VK_FORMAT_UNDEFINED;
		VkFormat mDepthFormat = VK_FORMAT_UNDEFINED;
		VkDeviceSize mImageSize = 0;
		VkImage mDepthImage = VK_NULL_HANDLE;
		VmaAllocation mDepthMemory = VK_NULL_HANDLE;
		VkImageView mDepthView = VK_NULL_HANDLE;

		std::vector<VkImage> mColorImages;
		std::vector<VmaAllocation> mColorMemories;
		std::vector<VkImageView> mColorViews;
	};
}
#endif