#pragma once
#include <Common/Util/Library.h>
#include <Common/Util/Memory.h>

// forward declarations
namespace Cosmos::Engine { class Application; }
namespace Cosmos::Renderer::Vulkan { class Buffer; }
namespace Cosmos::Renderer::Vulkan { class Device; }
namespace Cosmos::Renderer::Vulkan { class Instance; }
namespace Cosmos::Renderer::Vulkan { class Pipeline; }
namespace Cosmos::Renderer::Vulkan { class Renderpass; }
namespace Cosmos::Renderer::Vulkan { class Swapchain; }

namespace Cosmos::Renderer
{
	class Context
	{
	public:

		// delete copy constructor
		Context(const Context&) = delete;

		// delete assignment constructor
		Context& operator=(const Context&) = delete;

	public:

		// initializes the main window
		static void Initialize(Engine::Application* application);

		// terminates the main window
		static void Shutdown();

		// returns main window singleton instance
		static Context& GetRef();

	private:

		// constructor
		Context(Engine::Application* application);

		// destructor
		~Context();

	public:

		// returns what frame is currently on frame, since multiple frames are simultaneously rendered
		inline uint32_t GetCurrentFrame() const { return mCurrentFrame; }

		// returns the instance
		inline Shared<Vulkan::Instance> GetInstance() { return mInstance; }

		// returns the device
		inline Shared<Vulkan::Device> GetDevice() { return mDevice; }

		// returns the swapchain
		inline Shared<Vulkan::Swapchain> GetSwapchain() { return mSwapchain; }

		// return a reference to the main render pass at the momment
		inline Shared<Vulkan::Renderpass>& GetMainRenderpassRef() { return mMainRenderpass; }

		// returns a reference to the render passes
		inline Library<Shared<Vulkan::Renderpass>>& GetRenderpassesLibraryRef() { return mRenderpasses; }

		// returns a reference to the buffers
		inline Library<Shared<Vulkan::Buffer>>& GetBuffersLibraryRef() { return mBuffers; }

		// returns a reference to the pipelines
		inline Library<Shared<Vulkan::Pipeline>>& GetPipelinesLibraryRef() { return mPipelines; }

	public:

		// updates the renderer
		void OnUpdate();

		// pipeline creation/recreation is used when they're no longer valid
		void RecreatePipelines();

	private:

		// manage all active render passes
		void ManageRenderpasses(uint32_t swapchainImageIndex);

	public:

		Engine::Application* mApplication = nullptr;
		uint32_t mCurrentFrame = 0;
		uint32_t mImageIndex = 0;
		Shared<Vulkan::Instance> mInstance;
		Shared<Vulkan::Device> mDevice;
		Shared<Vulkan::Swapchain> mSwapchain;
		Shared<Vulkan::Renderpass> mMainRenderpass;
		Library<Shared<Vulkan::Renderpass>> mRenderpasses;
		Library<Shared<Vulkan::Buffer>> mBuffers;
		Library<Shared<Vulkan::Pipeline>> mPipelines;
	};
}