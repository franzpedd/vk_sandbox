#if defined RENDERER_VULKAN
#include "Mesh.h"

#include "Buffer.h"
#include "Device.h"
#include "Pipeline.h"
#include "Renderpass.h"
#include "Texture.h"
#include "Renderer/Core/Context.h"
#include "Wrapper/tinygltf.h"
#include <Common/Core/Defines.h>
#include <Common/Debug/Logger.h>
#include <Common/File/Filesystem.h>
#include <filesystem>

namespace Cosmos::Renderer::Vulkan
{
	Mesh::Mesh()
	{
		COSMOS_LOG(Logger::Trace, "Implement animation requests");
	}

	Mesh::~Mesh()
	{
		Clear();
	}

	void Mesh::OnUpdate(float timestep)
	{
		// this is not complete
		ProcessAnimation(timestep);
	}

	void Mesh::OnRender(const glm::mat4& transform, uint64_t id)
	{
		if (!mLoaded) {
			return;
		}

		Context& renderer = Context::GetRef();
		VkDeviceSize offsets[] = { 0 };
		VkCommandBuffer cmdBuffer = renderer.GetMainRenderpassRef()->GetCommandfuffersRef()[renderer.GetCurrentFrame()];
		VkPipelineLayout pipelineLayout = renderer.GetPipelinesLibraryRef().GetRef("Mesh")->GetPipelineLayout();
		VkPipeline pipeline = renderer.GetPipelinesLibraryRef().GetRef("Mesh")->GetPipeline();
		
		vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &mGPUData.vertexBuffer, offsets);
		vkCmdBindIndexBuffer(cmdBuffer, mGPUData.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &mGPUData.descriptorSets[renderer.GetCurrentFrame()], 0, NULL);
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		
		MeshBuffer constants = {};
		constants.id = id;
		constants.model = transform;
		vkCmdPushConstants(cmdBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(MeshBuffer), &constants);
		
		for (auto& node : mNodes) {
			RenderNode(node, cmdBuffer);
		}
	}

	bool Mesh::IsTransfering()
	{
		if (!mLoaded) {
			return false;
		}

		VkResult res = vkGetFenceStatus(Context::GetRef().GetDevice()->GetLogicalDevice(), mGPUData.transferFence);
		return res == VK_SUCCESS ? false : true;
	}

	void Mesh::LoadFromFile(std::string path, float scale)
	{
		Clear();

		tinygltf::Model model;
		tinygltf::TinyGLTF context;
		std::string error, warning;

		bool fileLoaded = context.LoadASCIIFromFile(&model, &error, &warning, path);

		if (!fileLoaded) {
			COSMOS_LOG(Logger::Error, "Failed to load mesh %s, error: %s", path.c_str(), error.c_str());
			return;
		}

		if (warning.size() > 0) {
			COSMOS_LOG(Logger::Warn, "Loading mesh %s with warning(s): %s", path.c_str(), warning.c_str());
		}
		
		// get vertex and index count
		size_t verticesCount = 0;
		size_t indicesCount = 0;
		const tinygltf::Scene& scene = model.scenes[model.defaultScene > -1 ? model.defaultScene : 0];
		
		for (size_t i = 0; i < scene.nodes.size(); i++) {
			GLTF::Node::GetNodeVertexAndIndexCount(model.nodes[scene.nodes[i]], model, verticesCount, indicesCount);
		}
		
		mVertices.resize(verticesCount);
		
		// load and parse gltf properties
		mName = std::filesystem::path(path).filename().string();
		mPath = path;

		mMaterial.GetNameRef() = "Default Material";
		mMaterial.GetAlbedoTextureRef() = CreateShared<Texture2D>(GetAssetSubDir("Texture/Default/default_1024_grey.png"));

		GLTF::Node::MeshLoaderInfo info = {};
		info.vertexBuffer = new Vertex[verticesCount];
		info.indexBuffer = new uint32_t[indicesCount];

		for (size_t i = 0; i < scene.nodes.size(); i++) {
			const tinygltf::Node node = model.nodes[scene.nodes[i]];
			GLTF::Node::LoadNode(nullptr, node, scene.nodes[i], model, info, mNodes, mLinearNodes, mMaterial, mVertices, scale);
		}

		mAnimations = GLTF::Animation::LoadAnimations(model, mNodes);
		mSkins = GLTF::Skin::LoadSkins(model, mNodes);

		// assign skins and initial positions
		for (auto node : mLinearNodes) {
			if (node->GetSkinIndex() > -1) {
				node->SetSkin(mSkins[node->GetSkinIndex()]);
			}

			if (node->GetMesh()) {
				node->OnUpdate();
			}
		}

		// gpu resources
		CreateRendererResources((uint32_t)verticesCount, (uint32_t)indicesCount, info);
		SetupDescriptors();
		UpdateDescriptors();

		// free resources
		delete[] info.vertexBuffer;
		delete[] info.indexBuffer;

		mLoaded = true;
	}

    void Mesh::Refresh()
    {
		UpdateDescriptors();
    }

    void Mesh::CreateRendererResources(uint32_t verticesCount, uint32_t indicesCount, GLTF::Node::MeshLoaderInfo& info)
    {
		size_t verticesBufferSize = verticesCount * sizeof(Vertex);
		size_t indicesBufferSize = indicesCount * sizeof(uint32_t);

		// create staging buffers
		struct StagingBuffer
		{
			VkBuffer buffer;
			VmaAllocation memory;
		} vertexStaging, indexStaging;

		Context& renderer = Context::GetRef();

		COSMOS_ASSERT(renderer.GetDevice()->CreateBuffer
		(
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			verticesBufferSize,
			&vertexStaging.buffer,
			&vertexStaging.memory,
			info.vertexBuffer) == VK_SUCCESS, "Failed to create vertex staging buffer"
		);

		if (indicesBufferSize > 0)
		{
			COSMOS_ASSERT(renderer.GetDevice()->CreateBuffer
			(
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				indicesBufferSize,
				&indexStaging.buffer,
				&indexStaging.memory,
				info.indexBuffer) == VK_SUCCESS, "Failed to create index staging buffer"
			);
		}

		// create local buffers
		COSMOS_ASSERT(renderer.GetDevice()->CreateBuffer
		(
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			verticesBufferSize,
			&mGPUData.vertexBuffer,
			&mGPUData.vertexMemory) == VK_SUCCESS, "Failed to create vertex local buffer"
		);

		if (indicesBufferSize > 0)
		{
			COSMOS_ASSERT(renderer.GetDevice()->CreateBuffer
			(
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				indicesBufferSize,
				&mGPUData.indexBuffer,
				&mGPUData.indexMemory) == VK_SUCCESS, "Failed to create index local buffer"
			);
		}

		// copy from staging buffer to device local buffer
		COSMOS_LOG(Logger::Info, "Send a signal/fence alongside the copy command in order to know when transfering is complete");

		auto& renderpass = renderer.GetMainRenderpassRef();
		VkCommandBuffer cmdbuffer = renderer.GetDevice()->CreateCommandBuffer(renderpass->GetCommandPoolRef(), VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

		VkBufferCopy copyRegion = {};
		copyRegion.size = verticesBufferSize;
		vkCmdCopyBuffer(cmdbuffer, vertexStaging.buffer, mGPUData.vertexBuffer, 1, &copyRegion);

		if (indicesBufferSize > 0) {
			copyRegion.size = indicesBufferSize;
			vkCmdCopyBuffer(cmdbuffer, indexStaging.buffer, mGPUData.indexBuffer, 1, &copyRegion);
		}

		COSMOS_ASSERT(vkEndCommandBuffer(cmdbuffer) == VK_SUCCESS, "Failed to end the recording of the command buffer");

		// submit to gpu
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdbuffer;

		VkFenceCreateInfo fenceCI = {};
		fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCI.pNext = nullptr;
		fenceCI.flags = 0;

		COSMOS_ASSERT(vkCreateFence(renderer.GetDevice()->GetLogicalDevice(), &fenceCI, nullptr, &mGPUData.transferFence) == VK_SUCCESS, "Failed to create fence for command buffer submission");
		COSMOS_ASSERT(vkQueueSubmit(renderer.GetDevice()->GetGraphicsQueue(), 1, &submitInfo, mGPUData.transferFence) == VK_SUCCESS, "Failed to submit command buffer");
		COSMOS_LOG(Logger::Info, "Todo: Command buffers deletion queue must be implemented, they're currently not being deleted. Make an assets manager to handle such things");

		// free staging resources
		vmaDestroyBuffer(renderer.GetDevice()->GetAllocator(), vertexStaging.buffer, vertexStaging.memory);

		if (indicesBufferSize > 0) {
			vmaDestroyBuffer(renderer.GetDevice()->GetAllocator(), indexStaging.buffer, indexStaging.memory);
		}
    }

    void Mesh::SetupDescriptors()
    {
		Context& renderer = Context::GetRef();

		// descriptor pool and descriptor sets
		std::array<VkDescriptorPoolSize, 2> poolSizes = {};
		// 0: Camera data
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = CONCURENTLY_RENDERED_FRAMES;
		// 1: Albedo map
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = CONCURENTLY_RENDERED_FRAMES;

		VkDescriptorPoolCreateInfo descPoolCI = {};
		descPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descPoolCI.poolSizeCount = (uint32_t)poolSizes.size();
		descPoolCI.pPoolSizes = poolSizes.data();
		descPoolCI.maxSets = CONCURENTLY_RENDERED_FRAMES;
		COSMOS_ASSERT(vkCreateDescriptorPool(renderer.GetDevice()->GetLogicalDevice(), &descPoolCI, nullptr, &mGPUData.descriptorPool) == VK_SUCCESS, "Failed to create descriptor pool");

		std::vector<VkDescriptorSetLayout> layouts(CONCURENTLY_RENDERED_FRAMES, renderer.GetPipelinesLibraryRef().GetRef("Mesh")->GetDescriptorSetLayout());

		VkDescriptorSetAllocateInfo descSetAllocInfo = {};
		descSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descSetAllocInfo.descriptorPool = mGPUData.descriptorPool;
		descSetAllocInfo.descriptorSetCount = (uint32_t)CONCURENTLY_RENDERED_FRAMES;
		descSetAllocInfo.pSetLayouts = layouts.data();

		mGPUData.descriptorSets.resize(CONCURENTLY_RENDERED_FRAMES);
		COSMOS_ASSERT(vkAllocateDescriptorSets(renderer.GetDevice()->GetLogicalDevice(), &descSetAllocInfo, mGPUData.descriptorSets.data()) == VK_SUCCESS, "Failed to allocate descriptor sets");
    }

    void Mesh::UpdateDescriptors()
    {
		Context& renderer = Context::GetRef();

		for (size_t i = 0; i < CONCURENTLY_RENDERED_FRAMES; i++) {

			// 0: Camera data
			{
				VkDescriptorBufferInfo bufferInfo = {};
				bufferInfo.buffer = renderer.GetBuffersLibraryRef().GetRef("Camera")->GetBuffersRef()[i];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(Renderer::Vulkan::CameraBuffer);

				VkWriteDescriptorSet cameraDesc = {};
				cameraDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				cameraDesc.dstSet = mGPUData.descriptorSets[i];
				cameraDesc.dstBinding = 0;
				cameraDesc.dstArrayElement = 0;
				cameraDesc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				cameraDesc.descriptorCount = 1;
				cameraDesc.pBufferInfo = &bufferInfo;

				vkUpdateDescriptorSets(renderer.GetDevice()->GetLogicalDevice(), 1, &cameraDesc, 0, nullptr);
			}

			// 1: Albedo map
			{
				VkDescriptorImageInfo colorMapInfo = {};
				colorMapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				colorMapInfo.imageView = (VkImageView)mMaterial.GetAlbedoTextureRef()->GetView();
				colorMapInfo.sampler = (VkSampler)mMaterial.GetAlbedoTextureRef()->GetSampler();

				VkWriteDescriptorSet colorMapDesc = {};
				colorMapDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				colorMapDesc.dstSet = mGPUData.descriptorSets[i];
				colorMapDesc.dstBinding = 4;
				colorMapDesc.dstArrayElement = 0;
				colorMapDesc.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				colorMapDesc.descriptorCount = 1;
				colorMapDesc.pImageInfo = &colorMapInfo;

				vkUpdateDescriptorSets(renderer.GetDevice()->GetLogicalDevice(), 1, &colorMapDesc, 0, nullptr);
			}
		}
    }

	void Mesh::Clear()
	{
		Context& renderer = Context::GetRef();
		vkDeviceWaitIdle(renderer.GetDevice()->GetLogicalDevice());
		
		if (mGPUData.transferFence != VK_NULL_HANDLE) {
			vkDestroyFence(renderer.GetDevice()->GetLogicalDevice(), mGPUData.transferFence, nullptr);
			mGPUData.transferFence = VK_NULL_HANDLE;
		}
		
		if (mGPUData.descriptorPool != VK_NULL_HANDLE) {
			vkDestroyDescriptorPool(renderer.GetDevice()->GetLogicalDevice(), mGPUData.descriptorPool, nullptr);
			mGPUData.descriptorPool = VK_NULL_HANDLE;
		}
		
		if (mGPUData.vertexBuffer != VK_NULL_HANDLE) {
			vmaDestroyBuffer(renderer.GetDevice()->GetAllocator(), mGPUData.vertexBuffer, mGPUData.vertexMemory);
			mGPUData.vertexBuffer = VK_NULL_HANDLE;
			mGPUData.vertexMemory = VK_NULL_HANDLE;
		}
		
		if (mGPUData.indexBuffer != VK_NULL_HANDLE) {
			vmaDestroyBuffer(renderer.GetDevice()->GetAllocator(), mGPUData.indexBuffer, mGPUData.indexMemory);
			mGPUData.indexBuffer = VK_NULL_HANDLE;
			mGPUData.indexMemory = VK_NULL_HANDLE;
		}
		
		for (auto skin : mSkins) {
			delete skin;
		}
		
		for (auto node : mNodes) {
			delete node;
		}
		
		mAnimations.resize(0);
		mSkins.resize(0);
		mNodes.resize(0);
		mLinearNodes.resize(0);
		mLoaded = false;
	}

	void Mesh::RenderNode(GLTF::Node* node, VkCommandBuffer commandBuffer)
	{
		if (node->GetMesh() != nullptr) {
			for (GLTF::Primitive* primitive : node->GetMesh()->GetPrimitivesRef()) {
				if (primitive->GetIndexCount() > 0) {
					vkCmdDrawIndexed(commandBuffer, primitive->GetIndexCount(), 1, primitive->GetFirstIndex(), 0, 0);
				}
			}
		}
		
		for (auto& child : node->GetChildrenRef()) {
			RenderNode(child, commandBuffer);
		}
	}

	void Mesh::ProcessAnimation(float timestep, int32_t index)
	{
		if (mAnimations.empty() || index < 0) {
			return;
		}

		if (index > (int32_t)(mAnimations.size() - 1)) {
			COSMOS_LOG(Logger::Error, "Mesh does not contain animation with index %d", index);
			return;
		}

		GLTF::Animation& animation = mAnimations[index];

		bool updated = false;

		for (auto& channel : animation.GetChannelsRef()) {
			GLTF::Animation::Sampler& sampler = animation.GetSamplersRef()[channel.samplerIndex];

			if (sampler.inputs.size() > sampler.outputsVec4.size()) {
				continue;
			}

			for (size_t i = 0; i < sampler.inputs.size() - 1; i++) {
				if ((timestep >= sampler.inputs[i]) && (timestep <= sampler.inputs[i + 1])) {
					float u = std::max(0.0f, timestep - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);

					if (u <= 1.0f) {
						switch (channel.path)
						{
							case GLTF::Animation::Channel::PathType::TRANSLATION:
							{
								sampler.Translate(i, timestep, channel.node);
								break;
							}

							case GLTF::Animation::Channel::PathType::SCALE:
							{
								sampler.Scale(i, timestep, channel.node);
								break;
							}

							case GLTF::Animation::Channel::PathType::ROTATION:
							{
								sampler.Rotate(i, timestep, channel.node);
								break;
							}
						}
						updated = true;
					}
				}
			}
		}

		if (updated) {
			for (auto& node : mNodes) {
				node->OnUpdate();
			}
		}
	}
}

#endif