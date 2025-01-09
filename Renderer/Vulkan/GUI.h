#pragma once
#ifdef RENDERER_VULKAN

#include "Core/IGUI.h"

namespace Cosmos::Renderer::Vulkan
{
	class GUI : public Renderer::IGUI
	{
	public:

		// constructor
		GUI();

		// destructor
		virtual ~GUI();

	public:

		// updates the ui logic
		virtual void OnUpdate() override;

		// draws the ui
		virtual void OnRender() override;

		// called when an event happens
		virtual void OnEvent(Shared<Platform::EventBase> event) override;

	public:

		// adds a texture to be used on the ui, only needs opaque pointers
		virtual void* AddTexture(void* sampler, void* view) override;

		// adds a texture to be used on the ui
		virtual void* AddTexture(Shared<ITexture2D> texture) override;

	public:

		// sets how many frames are simultaneously being rendered
		void SetImageCount(uint32_t count);

		// draws the renderer api data
		void DrawBackendData(void* commandBuffer);

	private:

		// create all used resouces by the renderer
		void CreateResources();

		// initializes the backend for the ui
		void SetupBackend();
	};
}

#endif