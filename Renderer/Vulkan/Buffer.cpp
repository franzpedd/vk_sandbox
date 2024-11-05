#if defined RENDERER_VULKAN
#include "Buffer.h"

#include "Device.h"
#include "Core/Defines.h"
#include <Common/Debug/Logger.h>

namespace Cosmos::Renderer::Vulkan
{
	Buffer::Buffer(Shared<Device> device, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceSize size)
		: mDevice(device)
	{
		mBuffers.resize(CONCURENTLY_RENDERED_FRAMES);
		mMemories.resize(CONCURENTLY_RENDERED_FRAMES);
		mMappedData.resize(CONCURENTLY_RENDERED_FRAMES);

		for (size_t i = 0; i < CONCURENTLY_RENDERED_FRAMES; i++)
		{
			COSMOS_ASSERT
			(
				mDevice->CreateBuffer
				(
					usage,
					properties,
					size,
					&mBuffers[i],
					&mMemories[i]
				) == VK_SUCCESS,
				"Failed to create buffer"
			);

			vmaMapMemory(mDevice->GetAllocator(), mMemories[i], &mMappedData[i]);
		}

		mMapped = true;
	}

	Buffer::~Buffer()
	{
		for (size_t i = 0; i < CONCURENTLY_RENDERED_FRAMES; i++)
		{
			if (mMapped) {
				vmaUnmapMemory(mDevice->GetAllocator(), mMemories[i]);
			}

			vmaDestroyBuffer(mDevice->GetAllocator(), mBuffers[i], mMemories[i]);
		}
	}

	void Buffer::Map()
	{
		for (size_t i = 0; i < CONCURENTLY_RENDERED_FRAMES; i++) {
			vmaMapMemory(mDevice->GetAllocator(), mMemories[i], &mMappedData[i]);
		}

		mMapped = true;
	}

	void Buffer::Unmap()
	{
		for (size_t i = 0; i < CONCURENTLY_RENDERED_FRAMES; i++) {
			vmaUnmapMemory(mDevice->GetAllocator(), mMemories[i]);
		}

		mMapped = false;
	}
}
#endif