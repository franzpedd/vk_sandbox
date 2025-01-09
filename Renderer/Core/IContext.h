#pragma once

#include <Common/Math/Math.h>
#include <Common/Util/Memory.h>

// forward declarations
namespace Cosmos::Engine { class Application; }
namespace Cosmos::Platform { class EventBase; }
namespace Cosmos::Renderer::Vulkan { class Context; }

namespace Cosmos::Renderer
{
	class IContext
	{
	public:

		enum Stage : uint32_t
		{
			Default = 0,
			Picking,
			Wireframe
		};

		struct ViewportBoundaries
		{
			glm::vec2 position = { 0.0f, 0.0f };
			glm::vec2 size = { 0.0f, 0.0f };
			glm::vec2 min = { 0.0f, 0.0f };
			glm::vec2 max = { 0.0f, 0.0f };
		};

	public:

		// returns wich frame is currently being processed, since multiple frames are simultaneously rendered
		inline unsigned int GetCurrentFrame() const { return mCurrentFrame; }

		// returns a reference to the viewport boundaries, wich holds size information about the main viewport
		inline ViewportBoundaries& GetViewportBoundariesRef() { return mViewportBoundaries; }

	public:

		// delete copy constructor
		IContext(const IContext&) = delete;

		// delete assignment constructor
		IContext& operator=(const IContext&) = delete;

		// initializes the graphic context
		static void Initialize(Engine::Application* application);

		// terminates the graphi context
		static void Shutdown();

		#ifdef RENDERER_VULKAN
		// returns the graphic context singleton, witchever is the active context at the time
		static IContext* GetRef();
		#else
		#error "Unsupported Renderer";
		#endif

	public:

		// called for updating the renderer
		virtual void OnUpdate() = 0;

		// called when an event happens
		virtual void OnEvent(Shared<Platform::EventBase> event) = 0;

	protected:

		// constructor
		IContext() = default;

		// destructor
		~IContext() = default;

	protected:

		Engine::Application* mApplication = nullptr;
		ViewportBoundaries mViewportBoundaries;
		unsigned int mCurrentFrame = 0;
	};
}