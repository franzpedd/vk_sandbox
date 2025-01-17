#pragma once
#ifdef RENDERER_VULKAN
#include <Renderer/Wrapper/vulkan.h>

#include <Common/Util/Memory.h>
#include <vector>

// forwar declarations
namespace Cosmos::Platform { class EventBase; }
namespace Cosmos::Renderer::Vulkan { class Shader; }

namespace Cosmos::Editor
{
	class Grid
	{
	public:

		// constructor
		Grid();

		// destructor
		~Grid();

		// returns if the grid is visible
		inline bool GetVisibility() { return mVisible; }

		// sets/unsets the grid visibility
		inline void SetVisibility(bool value) { mVisible = value; }

	public:

		// draws the grid
		void OnRender();

		// called when an event happen
		void OnEvent(Shared<Platform::EventBase> event);

	public:

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