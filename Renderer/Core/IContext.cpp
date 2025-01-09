#include "IContext.h"

#include <Common/Debug/Logger.h>
#include "Vulkan/Context.h"

namespace Cosmos::Renderer
{
	#ifdef RENDERER_VULKAN
	static Vulkan::Context* s_Instance = nullptr;
	#else
	#error "Unsupported Renderer";
	#endif

	void IContext::Initialize(Engine::Application* application)
	{
		if (s_Instance) {
			COSMOS_LOG(Logger::Error, "Attempting to initialize the Graphics Context while it's active");
			return;
		}

		#ifdef RENDERER_VULKAN
		s_Instance = new Vulkan::Context(application);
		#endif
	}

	void IContext::Shutdown()
	{
		delete s_Instance;
		s_Instance = nullptr;
	}

	IContext* IContext::GetRef()
	{
		if (!s_Instance) {
			COSMOS_LOG(Logger::Error, "The graphics context has not been initialized\n");
		}

		return s_Instance;
	}
}