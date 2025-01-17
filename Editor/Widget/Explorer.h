#pragma once
#include <Renderer/Wrapper/imgui.h>
#include <Renderer/Wrapper/vulkan.h>

#include <Renderer/Core/IGUI.h>
#include <Renderer/GUI/Widget.h>

#include <array>
#include <vector>
#include <string>

// forward declarations
namespace Cosmos::Renderer::Vulkan { class Texture2D; }
namespace Cosmos::Editor { class Application; }

namespace Cosmos::Editor
{
	class Asset
	{
	public:

		enum Type : uint32_t
		{
			Undefined = 0,
			Folder,
			Text,
			Scene,
			Vert,
			Frag,
			Spv,
			Mesh,
			Sound,
			Image,

			ASSET_TYPE_MAX
		};

		struct ViewResource
		{
			Shared<Renderer::Vulkan::Texture2D> texture = {};
			VkDescriptorSet descriptor = VK_NULL_HANDLE;
		};

		Type type = Type::Undefined;
		ViewResource view = {};
		std::string path = {};
		std::string name = {};
	};
	
	class Explorer : public Renderer::Widget
	{
	public:

		// constructor
		Explorer(Application* application);

		// destructor
		virtual ~Explorer();

	public:

		// updates the tick logic
		virtual void OnUpdate() override;

	public:

		// sets on/off the debug window
		inline void SetOpened(bool value) { mOpened = value; }

		// returns if the debug window is opened
		inline bool IsOpened() { return mOpened; }

		// signals the explorer to be refreshed, usually used by external classes
		inline void HintRefresh() { mRefreshExplorer = true; }

	private:

		// draws a folder in the explorer
		void DrawAsset(Asset& asset, ImVec2& position, const ImVec2 buttonSize);

		// reloads the folder's content
		void Refresh(std::string path);

		// draws a menu if right mouse is clicked on the window
		void DisplayRightClickMenu();
		
		// draws a menu if right mouse is pressed on item
		void DisplayAssetMenu(Asset& asset);

	private:

		Application* mApplication = nullptr;
		bool mOpened = true;
		bool mRecursiveSearch = false;
		bool mRefreshExplorer = true;
		std::string mCurrentDir = {};
		std::vector<Asset> mCurrentDirAssets = {};
		std::string mSearchboxText = {};

		// default resources
		std::array<Asset, Asset::ASSET_TYPE_MAX> mAssets;
		std::array<std::string, Asset::ASSET_TYPE_MAX> mAssetsPath = {};
		Asset mParentFolder;
	};
}