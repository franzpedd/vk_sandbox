#pragma once
#if defined RENDERER_VULKAN

#include "Core/IMesh.h"
#include "Core/Vertex.h"
#include "GLTF/Animation.h"
#include "GLTF/Node.h"
#include "GLTF/Mesh.h"
#include "GLTF/Skin.h"
#include "Wrapper/vulkan.h"
#include <string>
#include <vector>

namespace Cosmos::Renderer::Vulkan
{ 
	class Mesh : public Cosmos::Renderer::IMesh
	{
	public:

		struct Dimension
		{
			glm::mat4 aabb;
			glm::vec3 min = glm::vec3(FLT_MAX);
			glm::vec3 max = glm::vec3(-FLT_MAX);
		};

		struct GPUData
		{
			VkBuffer vertexBuffer = VK_NULL_HANDLE;
			VmaAllocation vertexMemory = VK_NULL_HANDLE;
			VkBuffer indexBuffer = VK_NULL_HANDLE;
			VmaAllocation indexMemory = VK_NULL_HANDLE;
			VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
			std::vector<VkDescriptorSet> descriptorSets = {};
			VkFence transferFence = VK_NULL_HANDLE;
		};

	public:

		// constructor
		Mesh();

		// destructor
		virtual ~Mesh();

	public:

		// updates the mesh frame-logic
		virtual void OnUpdate(float timestep) override;

		// renders the mesh
		virtual void OnRender(const glm::mat4& transform, uint64_t id, IContext::Stage stage) override;

	public:

		// returns if mesh is currently being transfered
		virtual bool IsTransfering() override;

		// loads the mesh from it's filepath
		virtual void LoadFromFile(std::string path, float scale = 1.0f) override;

		// refreshes mesh configuration, applying any changes made
		virtual void Refresh() override;

	private:

		// creates all used resources by the renderer api
		void CreateRendererResources(uint32_t verticesCount, uint32_t indicesCount, GLTF::Node::MeshLoaderInfo& info);

		// configures the descriptors used by the mesh
		void SetupDescriptors();

		// sends the descriptor data into the gpu
		void UpdateDescriptors();

		// clears the resoruces used by the mesh, usefull when reloading another mesh
		void Clear();

		// draws a particular node
		void RenderNode(GLTF::Node* node, VkCommandBuffer commandBuffer);

		// updates the animation requests
		void ProcessAnimation(float timestep, int32_t index = -1);

	private:

		// gltf and mesh data
		GPUData mGPUData;
		std::vector<Vertex> mVertices = {};
		std::vector<GLTF::Node*> mNodes = {};
		std::vector<GLTF::Node*> mLinearNodes = {};
		std::vector<GLTF::Skin*> mSkins = {};
		std::vector<GLTF::Animation> mAnimations = {};
	};
}

#endif