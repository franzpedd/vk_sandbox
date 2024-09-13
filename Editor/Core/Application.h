#pragma once

#include <Engine/Core/Application.h>

// forward declaration
namespace Cosmos::Engine { class Project; }
namespace Cosmos::Editor { class Dockspace; }
namespace Cosmos::Editor { class Mainmenu; }

namespace Cosmos::Editor
{
	class Application : public Engine::Application
	{
	public:

		// constructor
		Application(Shared<Engine::Project> project);

		// destructor
		virtual ~Application();

	private:

		Dockspace* mDockspace = nullptr;
		Mainmenu* mMainmenu = nullptr;
	};
}