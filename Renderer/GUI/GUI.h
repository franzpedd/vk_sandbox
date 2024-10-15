#pragma once

#include <Common/Math/Math.h>
#include <Common/Util/Memory.h>
#include <Common/Util/Stack.h>

// forward declaration
union SDL_Event;
namespace Cosmos::Platform { class EventBase; }
namespace Cosmos::Renderer { class Widget; }
namespace Cosmos::Renderer::Vulkan { class Texture2D; }

namespace Cosmos::Renderer
{
	class GUI
	{
	public:

		// delete copy constructor
		GUI(const GUI&) = delete;

		// delete assignment constructor
		GUI& operator=(const GUI&) = delete;

	public:

		// initializes the main window
		static void Initialize();

		// terminates the main window
		static void Shutdown();

		// returns main window singleton instance
		static GUI& GetRef();

	private:

		// constructor
		GUI();

		// destructor
		~GUI();

	public:

		// returns a reference to the library of widgets
		inline Stack<Widget*>& GetWidgetsStackRef() { return mWidgets; }

	public:

		// updates the ui logic
		void OnUpdate();

		// draws the ui
		void OnRender();

		// called when an event happens
		void OnEvent(Shared<Platform::EventBase> event);

		// toggles the cursor on and off
		void ToggleCursor(bool hide);

		// adds a new widget to the ui
		void AddWidget(Widget* widget);

	public:

		// adds a texture to be used on the ui
		void* AddTexture(Shared<Vulkan::Texture2D> texture);

		// adds a texture to be used on the ui, only needs opaque pointers
		void* AddTexture(void* sampler, void* view);

		// sets how many frames are simultaneously being rendered
		void SetImageCount(uint32_t count);

		// draws the renderer api data
		void DrawBackendData(void* commandBuffer);

	private:

		// create all used resouces by the renderer
		void CreateResources();

		// initializes the backend for the ui
		void SetupBackend();

	private:

		Stack<Widget*> mWidgets;
	};
}