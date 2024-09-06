#include "Application.h"

#include "GUI/GUI.h"
#include "Widget/Mainmenu.h"
#include "Viewport/Grid.h"

namespace Cosmos::Editor
{
	Application::Application()
	{
		mGrid = new Grid();
		Renderer::GUI::GetRef().AddWidget(mGrid);

		mMainmenu = new Mainmenu(this);
		Renderer::GUI::GetRef().AddWidget(mMainmenu);
	}

	Application::~Application()
	{
	}
}