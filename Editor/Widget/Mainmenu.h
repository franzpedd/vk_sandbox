#pragma once

#include <Renderer/GUI/Widget.h>

// forward declarations
namespace Cosmos::Editor { class Application; }
namespace Cosmos::Editor { class DebugWindow; }
namespace Cosmos::Editor { class Explorer; }
namespace Cosmos::Editor { class ImDemo; }
namespace Cosmos::Editor { class Viewport; }
namespace Cosmos::Editor { class Hierarchy; }

namespace Cosmos::Editor
{
	class Mainmenu : public Renderer::Widget
	{
	public:

		enum MenuAction 
		{
			None = 0,
			SceneNew, SceneOpen, SceneSave, SceneSaveAs
		};

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

		// handle menu popups if a menu action was issued
		void HandleMenuAction();

	private:

		Application* mApplication = nullptr;
		DebugWindow* mDebugWindow = nullptr;
		Explorer* mExplorer = nullptr;
		ImDemo* mImDemo = nullptr;
		Viewport* mViewport = nullptr;
		Hierarchy* mHierarchy = nullptr;

		MenuAction mMenuAction = MenuAction::None;
	};
}