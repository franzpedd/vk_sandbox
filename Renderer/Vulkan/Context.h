#pragma once
#ifdef RENDERER_VULKAN

#include "Core/IContext.h"
#include "Wrapper/vulkan.h"

#include <Common/Util/Library.h>
#include <Common/Util/Memory.h>

// forward declarations
namespace Cosmos::Renderer::Vulkan { class Buffer; }
namespace Cosmos::Renderer::Vulkan { class Device; }
namespace Cosmos::Renderer::Vulkan { class Instance; }
namespace Cosmos::Renderer::Vulkan { class Pipeline; }
namespace Cosmos::Renderer::Vulkan { class Picking; }
namespace Cosmos::Renderer::Vulkan { class Renderpass; }
namespace Cosmos::Renderer::Vulkan { class Swapchain; }

namespace Cosmos::Renderer::Vulkan
{
	class Context : public Renderer::IContext
	{
	public:

		// constructor
		Context(Engine::Application* application);

		// destructor
		virtual ~Context() = default;

	public:

		// returns the instance
		inline Shared<Vulkan::Instance> GetInstance() { return mInstance; }

		// returns the device
		inline Shared<Vulkan::Device> GetDevice() { return mDevice; }

		// returns the swapchain
		inline Shared<Vulkan::Swapchain> GetSwapchain() { return mSwapchain; }

		// return a reference to the main render pass at the momment
		inline Shared<Vulkan::Renderpass>& GetMainRenderpassRef() { return mMainRenderpass; }

		// returns a reference to the picking functionality
		inline Shared<Vulkan::Picking>& GetPickingRef() { return mPicking; }

		// returns a reference to the render passes
		inline Library<Shared<Vulkan::Renderpass>>& GetRenderpassesLibraryRef() { return mRenderpasses; }

		// returns a reference to the buffers
		inline Library<Shared<Vulkan::Buffer>>& GetBuffersLibraryRef() { return mBuffers; }

		// returns a reference to the pipelines
		inline Library<Shared<Vulkan::Pipeline>>& GetPipelinesLibraryRef() { return mPipelines; }

	public:

		// called for updating the renderer
		virtual void OnUpdate() override;

		// called when an event happens
		virtual void OnEvent(Shared<Platform::EventBase> event) override;

	private:

		// make all necessary draw-calls
		void ManageRenderpasses(uint32_t swapchainImageIndex);

	private:

		Shared<Vulkan::Instance> mInstance;
		Shared<Vulkan::Device> mDevice;
		Shared<Vulkan::Swapchain> mSwapchain;
		Shared<Vulkan::Renderpass> mMainRenderpass;
		Shared<Vulkan::Picking> mPicking;
		Library<Shared<Vulkan::Renderpass>> mRenderpasses;
		Library<Shared<Vulkan::Buffer>> mBuffers;
		Library<Shared<Vulkan::Pipeline>> mPipelines;
	};
}

#endif