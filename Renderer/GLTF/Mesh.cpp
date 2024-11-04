#include "Mesh.h"

#include "Renderer/Vulkan/Device.h"
#include <Common/Debug/Logger.h>

namespace Cosmos::Renderer::GLTF
{
	Mesh::Mesh(Shared<Renderer::Vulkan::Device> device, glm::mat4 matrix)
		: mDevice(device)
	{
		COSMOS_LOG(Logger::Info, "Abstract Device away");

		mUniformBlock.matrix = matrix;

		COSMOS_ASSERT
		(
			mDevice->CreateBuffer
			(
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				sizeof(UniformBlock), // we're passing enough memory for the mesh matrices
				&mUniformBuffer.buffer,
				&mUniformBuffer.memory,
				&mUniformBlock
			) == VK_SUCCESS, "Failed to create Unfiform Buffer"
		);

		COSMOS_ASSERT(vmaMapMemory(mDevice->GetAllocator(), mUniformBuffer.memory, &mUniformBuffer.mappedMemoryPtr) == VK_SUCCESS, "Failed to map memory");
		
		mUniformBuffer.descriptorInfo.buffer = mUniformBuffer.buffer;
		mUniformBuffer.descriptorInfo.offset = 0;
		mUniformBuffer.descriptorInfo.range = sizeof(UniformBlock);
	}

	Mesh::~Mesh()
	{
		COSMOS_LOG(Logger::Info, "Implement deletion queue");

		vkDeviceWaitIdle(mDevice->GetLogicalDevice());
		
		vmaUnmapMemory(mDevice->GetAllocator(), mUniformBuffer.memory);
		vmaDestroyBuffer(mDevice->GetAllocator(), mUniformBuffer.buffer, mUniformBuffer.memory);
		
		for (Primitive* p : mPrimitives) {
			delete p;
		}
	}

	void Mesh::SetBoundingBox(glm::vec3 min, glm::vec3 max)
	{
		mBB.SetMin(min);
		mBB.SetMax(max);
		mBB.SetValid(true);
	}
}