#pragma once

#include <Engine/Core/Application.h>

// forward declaration

namespace Cosmos::Editor { class Grid; }
namespace Cosmos::Editor { class Mainmenu; }

namespace Cosmos::Editor
{
	class Application : public Engine::Application
	{
	public:

		// constructor
		Application();

		// destructor
		virtual ~Application();

	private:

		Grid* mGrid = nullptr;
		Mainmenu* mMainmenu = nullptr;
	};
}