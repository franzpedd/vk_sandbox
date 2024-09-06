#pragma once
#ifdef RENDERER_VULKAN
#include <Renderer/Wrapper/vulkan.h>

#include <Common/Util/Memory.h>
#include <Renderer/GUI/Widget.h>
#include <vector>

// forwar declarations
namespace Cosmos::Platform { class EventBase; }
namespace Cosmos::Renderer::Vulkan { class Shader; }


namespace Cosmos::Editor
{
	class Grid : public Renderer::Widget
	{
	public:

		// constructor
		Grid();

		// destructor
		virtual ~Grid();

	public:

		// draws the grid
		virtual void OnRender() override;

		// called when an event happen
		virtual void OnEvent(Shared<Platform::EventBase> event) override;

	public:

		// toogles on/off if grid should be drawn
		void ToogleOnOff();

		// create all renderer resources
		void CreateRendererResources();

	private:

		bool mVisible = true;

		Shared<Renderer::Vulkan::Shader> mVertexShader;
		Shared<Renderer::Vulkan::Shader> mFragmentShader;

		VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> mDescriptorSets;
	};
}
#endif