#include "Application.h"

#include "Widget/Dockspace.h"
#include "Widget/Mainmenu.h"
#include <Common/Debug/Logger.h>
#include <Renderer/GUI/GUI.h>

namespace Cosmos::Editor
{
	Application::Application(Shared<Engine::Project> project)
		: Engine::Application(project)
	{
		mDockspace = new Dockspace();
		Renderer::GUI::GetRef().AddWidget(mDockspace);

		mMainmenu = new Mainmenu(this);
		Renderer::GUI::GetRef().AddWidget(mMainmenu);
	}

	Application::~Application()
	{
	}
}