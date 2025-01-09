#include "Application.h"

#include "Widget/Dockspace.h"
#include "Widget/Mainmenu.h"
#include <Common/Debug/Logger.h>
#include <Renderer/Core/IGUI.h>

namespace Cosmos::Editor
{
	Application::Application(Shared<Engine::Project> project)
		: Engine::Application(project)
	{
		mDockspace = new Dockspace();
		Renderer::IGUI::GetRef()->AddWidget(mDockspace);

		mMainmenu = new Mainmenu(this);
		Renderer::IGUI::GetRef()->AddWidget(mMainmenu);
	}

	Application::~Application()
	{
	}
}