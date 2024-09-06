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
		alignas(16) glm::mat4 view = glm::mat4(1.0f);
		alignas(16) glm::mat4 projection = glm::mat4(1.0f);
		alignas(16) glm::mat4 viewProjection = glm::mat4(1.0f);
		alignas(16) glm::vec3 cameraFront = glm::vec3(1.0f);
	};

	// information the renderer needs to know about the mesh (sent via push constant)
	struct MeshBuffer
	{
		alignas(8) uint64_t id = 0;
		alignas(16) glm::mat4 model = glm::mat4(1.0f);
	};
}
#endif