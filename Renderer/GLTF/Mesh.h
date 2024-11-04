#pragma once

#include "Primitive.h"
#include "Wrapper/vulkan.h"
#include <Common/Core/Defines.h>
#include <Common/Math/Math.h>
#include <Common/Math/BoundingBox.h>
#include <Common/Util/Memory.h>
#include <vector>

// forward declaration
namespace Cosmos::Renderer { struct Vertex; }
namespace Cosmos::Renderer::Vulkan { class Device; }

namespace Cosmos::Renderer::GLTF
{
	class Mesh
	{
	public:

		#if defined RENDERER_VULKAN

		struct UniformBuffer
		{
			VkBuffer buffer;
			VmaAllocation memory;
			VkDescriptorBufferInfo descriptorInfo;
			VkDescriptorSet descriptorSet;
			void* mappedMemoryPtr = nullptr;
		};

		#endif

		struct UniformBlock
		{
			glm::mat4 matrix = glm::mat4(1.0f);
			glm::mat4 jointsMatrix[COSMOS_MESH_MAX_JOINTS] = {};
			uint32_t jointsCount = 0;
		};

	public:

		// constructor
		Mesh(Shared<Renderer::Vulkan::Device> device, glm::mat4 matrix);

		// destructor
		~Mesh();

		// returns a reference to the mesh uniform buffer
		inline UniformBuffer& GetUniformBufferRef() { return mUniformBuffer; }

		// returns a reference to the mesh uniform block
		inline UniformBlock& GetUniformBlockRef() { return mUniformBlock; }

		// returns a reference to the mesh bounding box
		inline BoundingBox& GetBoundingBoxRef() { return mBB; }

		// returns a reference to the mesh primitives
		inline std::vector<Primitive*>& GetPrimitivesRef() { return mPrimitives; }

	public:

		// sets the meshe's bounding box
		void SetBoundingBox(glm::vec3 min, glm::vec3 max);

	private:

		Shared<Renderer::Vulkan::Device> mDevice;
		UniformBuffer mUniformBuffer = {};
		UniformBlock mUniformBlock = {};
		BoundingBox mBB = {};
		std::vector<Primitive*> mPrimitives = {};
	};
}