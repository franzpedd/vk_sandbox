#pragma once

#include <Renderer/GUI/Widget.h>

// forward declarations
namespace Cosmos::Editor { class Application; }
namespace Cosmos::Editor { class DebugWindow; }
namespace Cosmos::Editor { class Explorer; }
namespace Cosmos::Editor { class ImDemo; }
namespace Cosmos::Editor { class Viewport; }
namespace Cosmos::Editor { class PrefabHierarchy; }

namespace Cosmos::Editor
{
	class Mainmenu : public Renderer::Widget
	{
	public:

		// constructor
		Mainmenu(Application* application);

		// destructor
		~Mainmenu();

	public:

		// user interface updating
		void OnUpdate();

	private:

		// display each main menu item
		void DisplayMenuItems();

	private:

		Application* mApplication = nullptr;
		DebugWindow* mDebugWindow = nullptr;
		Explorer* mExplorer = nullptr;
		ImDemo* mImDemo = nullptr;
		Viewport* mViewport = nullptr;
		PrefabHierarchy* mPrefabHierarchy = nullptr;
	};
}