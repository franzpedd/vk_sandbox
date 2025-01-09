#pragma once
#if defined RENDERER_VULKAN

#include "Wrapper/vulkan.h"

#include <Common/Math/Math.h>
#include <Common/Util/Memory.h>
#include <vector>

// forward declarations
namespace Cosmos::Renderer::Vulkan { class Device; }

namespace Cosmos::Renderer::Vulkan
{
	class Buffer
	{
	public:

		// constructor
		Buffer(Shared<Device> device, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceSize size); // size is sizeof('struct')

		// destructor
		~Buffer();

		// returns a reference to the gpu-mapped (note: if not mapped, using this will throw errors)
		inline std::vector<void*>& GetMappedDataRef() { return mMappedData; }

		// returns a reference to the cpu buffers
		inline std::vector<VkBuffer>& GetBuffersRef() { return mBuffers; }

	public:

		// allows reading the buffer from the gpu
		void Map();

		// disallows reading the buffer from the gpu
		void Unmap();

	private:

		Shared<Device> mDevice;
		bool mMapped = false;
		std::vector<VkBuffer> mBuffers = {};
		std::vector<VmaAllocation> mMemories = {};
		std::vector<void*> mMappedData = {};
	};

	// information the renderer needs to know about the camera
	struct CameraBuffer
	{
		alignas(8) glm::vec2 mousepos = glm::vec2(0.0f);		// holds the current mouse position
		alignas(16) glm::mat4 view = glm::mat4(1.0f);			// holds the view matrix
		alignas(16) glm::mat4 projection = glm::mat4(1.0f);		// holds the projection matrix
	};

	// information the renderer needs to know via push constant
	struct PushConstant
	{
		alignas(4) uint32_t selected = 0;						// marks if the object is selected
		alignas(8) uint64_t id = 0;								// holds the unique identifier of the object
		alignas(16) glm::mat4 model = glm::mat4(1.0f);			// holds the model matrix of the object
	};
}
#endif