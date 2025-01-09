#include "Grid.h"

#include <Common/Core/Defines.h>
#include <Common/Debug/Logger.h>
#include <Common/File/Filesystem.h>
#include <Engine/Entity/Camera.h>
#include <Platform/Core/Input.h>
#include <Platform/Core/MainWindow.h>
#include <Platform/Event/KeyboardEvent.h>
#include <Renderer/Core/IContext.h>
#include <Renderer/Core/IGUI.h>
#include <Renderer/Vulkan/Buffer.h>
#include <Renderer/Vulkan/Context.h>
#include <Renderer/Vulkan/Device.h>
#include <Renderer/Vulkan/Pipeline.h>
#include <Renderer/Vulkan/Renderpass.h>
#include <Renderer/Vulkan/Shader.h>

namespace Cosmos::Editor
{
	Grid::Grid()
	{
		CreateRendererResources();
	}

	Grid::~Grid()
	{
		Renderer::Vulkan::Context* renderer = (Renderer::Vulkan::Context*)(Renderer::IContext::GetRef());
		auto device = renderer->GetDevice();
		vkDeviceWaitIdle(device->GetLogicalDevice());

		vkDestroyDescriptorPool(device->GetLogicalDevice(), mDescriptorPool, nullptr);
	}

	void Grid::OnRender()
	{
		if (!mVisible) return;

		Renderer::Vulkan::Context* renderer = (Renderer::Vulkan::Context*)(Renderer::IContext::GetRef());
		uint32_t currentFrame = renderer->GetCurrentFrame();
		VkDeviceSize offsets[] = { 0 };
		VkCommandBuffer cmdBuffer = renderer->GetMainRenderpassRef()->GetCommandfuffersRef()[currentFrame];
		
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->GetPipelinesLibraryRef().GetRef("Grid")->GetPipeline());
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->GetPipelinesLibraryRef().GetRef("Grid")->GetPipelineLayout(), 0, 1, &mDescriptorSets[currentFrame], 0, nullptr);
		vkCmdDraw(cmdBuffer, 6, 1, 0, 0);
	}

	void Grid::OnEvent(Shared<Platform::EventBase> event)
	{
		if (event->GetType() == Platform::EventType::KeyboardPress)
		{
			auto& camera = Engine::Camera::GetRef();
			auto castedEvent = std::dynamic_pointer_cast<Platform::KeyboardPressEvent>(event);
			Platform::Keycode key = castedEvent->GetKeycode();

			// toggle editor viewport camera
			if (key == Platform::KEY_Z) {
				if (camera.CanMove() && camera.GetType() == Engine::CameraType::FreeLook) {
					camera.SetMove(false);
					Platform::MainWindow::GetRef().ToggleCursor(false);
					Renderer::IGUI::GetRef()->ToggleCursor(false);
				}

				else if (!camera.CanMove() && camera.GetType() == Engine::CameraType::FreeLook) {
					camera.SetMove(true);
					Platform::MainWindow::GetRef().ToggleCursor(true);
					Renderer::IGUI::GetRef()->ToggleCursor(true);
				}
			}
		}
	}

	void Grid::CreateRendererResources()
	{
		Renderer::Vulkan::Context* renderer = (Renderer::Vulkan::Context*)(Renderer::IContext::GetRef());

		// create descriptor pool and descriptor sets
		{
			VkDescriptorPoolSize poolSize = {};
			poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSize.descriptorCount = CONCURENTLY_RENDERED_FRAMES;

			VkDescriptorPoolCreateInfo descPoolCI = {};
			descPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descPoolCI.poolSizeCount = 1;
			descPoolCI.pPoolSizes = &poolSize;
			descPoolCI.maxSets = CONCURENTLY_RENDERED_FRAMES;
			COSMOS_ASSERT(vkCreateDescriptorPool(renderer->GetDevice()->GetLogicalDevice(), &descPoolCI, nullptr, &mDescriptorPool) == VK_SUCCESS, "Failed to create descriptor pool");

			std::vector<VkDescriptorSetLayout> layouts(CONCURENTLY_RENDERED_FRAMES, renderer->GetPipelinesLibraryRef().GetRef("Grid")->GetDescriptorSetLayout());

			VkDescriptorSetAllocateInfo descSetAllocInfo = {};
			descSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descSetAllocInfo.descriptorPool = mDescriptorPool;
			descSetAllocInfo.descriptorSetCount = CONCURENTLY_RENDERED_FRAMES;
			descSetAllocInfo.pSetLayouts = layouts.data();

			mDescriptorSets.resize(CONCURENTLY_RENDERED_FRAMES);
			COSMOS_ASSERT(vkAllocateDescriptorSets(renderer->GetDevice()->GetLogicalDevice(), &descSetAllocInfo, mDescriptorSets.data()) == VK_SUCCESS, "Failed to allocate descriptor sets");

			for (size_t i = 0; i < CONCURENTLY_RENDERED_FRAMES; i++)
			{
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = renderer->GetBuffersLibraryRef().GetRef("Camera")->GetBuffersRef()[i];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(Renderer::Vulkan::CameraBuffer);

				VkWriteDescriptorSet descriptorWrite{};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = mDescriptorSets[i];
				descriptorWrite.dstBinding = 0;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pBufferInfo = &bufferInfo;

				vkUpdateDescriptorSets(renderer->GetDevice()->GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);
			}
		}
	}
}