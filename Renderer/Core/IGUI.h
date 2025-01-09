#pragma once

#include <Common/Util/Memory.h>
#include <Common/Util/Stack.h>

// forward declarations
namespace Cosmos::Platform { class EventBase; }
namespace Cosmos::Renderer { class ITexture2D; }
namespace Cosmos::Renderer { class Widget; }
namespace Cosmos::Renderer::Vulkan { class GUI; }

namespace Cosmos::Renderer
{
	class IGUI
	{
	public:

		// returns a reference to the library of widgets
		inline Stack<Widget*>& GetWidgetsStackRef() { return mWidgets; }

	public:

		// delete copy constructor
		IGUI(const IGUI&) = delete;

		// delete assignment constructor
		IGUI& operator=(const IGUI&) = delete;

		// initializes the graphic context
		static void Initialize();

		// terminates the graphi context
		static void Shutdown();

		#ifdef RENDERER_VULKAN
		// returns the graphic context singleton, witchever is the active context at the time
		static IGUI* GetRef();
		#else
		#error "Unsupported Renderer";
		#endif

	protected:

		// constructor
		IGUI() = default;

		// destructor
		~IGUI() = default;

	public:

		// updates the ui logic
		virtual void OnUpdate() = 0;

		// draws the ui
		virtual void OnRender() = 0;

		// called when an event happens
		virtual void OnEvent(Shared<Platform::EventBase> event) = 0;

	public:

		// toggles the cursor on and off
		void ToggleCursor(bool hide);

		// adds a new widget to the ui
		void AddWidget(Widget* widget);

		// returns the first widget found given a name, nullptr otherwise
		Widget* FindWidgetByName(const char* name);

	public:

		// adds a texture to be used on the ui, only needs opaque pointers
		virtual void* AddTexture(void* sampler, void* view) = 0;

		// adds a texture to be used on the ui
		virtual void* AddTexture(Shared<ITexture2D> texture) = 0;

	protected:

		Stack<Widget*> mWidgets;
	};
}