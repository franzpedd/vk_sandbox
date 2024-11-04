#pragma once

#include <Common/Util/Memory.h>
#include <Renderer/GUI/Widget.h>
#include <Wrapper/imgui.h>
#include <Wrapper/vulkan.h>

#include <vector>

// forward declarations
namespace Cosmos::Platform { class EventBase; }
namespace Cosmos::Editor { class Gizmos; }
namespace Cosmos::Editor { class Grid; }
namespace Cosmos::Editor { class PrefabHierarchy; }

namespace Cosmos::Editor
{
	class Viewport : public Renderer::Widget
	{
	public:

		// constructor
		Viewport(PrefabHierarchy* prefabHierarchy);

		// destructor
		virtual ~Viewport();

	public:

		// for user interface drawing
		virtual void OnUpdate();

		// for renderer drawing
		virtual void OnRender();

		// // called when the window is resized
		virtual void OnEvent(Shared<Platform::EventBase> event);

	private:

		// draws a menubar into the viewport, for gizmos operation and more
		void DrawMenu();

		// creates all renderer resources
		void CreateRendererResources();

		// creates all framebuffer resources
		void CreateFramebufferResources();

	private:

		PrefabHierarchy* mPrefabHierarchy;

		// sub-widgets
		Unique<Gizmos> mGizmos;
		Unique<Grid> mGrid;

		// widget boundaries
		ImVec2 mViewportPos;
		ImVec2 mCurrentSize;
		ImVec2 mContentRegionMin;
		ImVec2 mContentRegionMax;

		// vulkan resources
		VkFormat mSurfaceFormat = VK_FORMAT_UNDEFINED;
		VkFormat mDepthFormat = VK_FORMAT_UNDEFINED;
		VkSampler mSampler = VK_NULL_HANDLE;

		VkImage mDepthImage = VK_NULL_HANDLE;
		VmaAllocation mDepthMemory = VK_NULL_HANDLE;
		VkImageView mDepthView = VK_NULL_HANDLE;

		std::vector<VkImage> mImages;
		std::vector<VmaAllocation> mImageMemories;
		std::vector<VkImageView> mImageViews;
		std::vector<VkDescriptorSet> mDescriptorSets;
	};
}