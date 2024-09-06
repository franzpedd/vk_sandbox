#include "Application.h"

#include "Timestep.h"
#include "Entity/Camera.h"
#include <Platform/Core/MainWindow.h>
#include <Renderer/Core/Context.h>
#include <Renderer/GUI/GUI.h>

namespace Cosmos::Engine
{
	Application::Application()
	{
		Platform::MainWindow::Initialize(this, "Application", 800, 600, false);
		Camera::Initialize(Platform::MainWindow::GetRef().GetAspectRatio());
		Renderer::Context::Initialize(this);
		Renderer::GUI::Initialize();

		mTimestep = CreateUnique<Timestep>(this);
	}

	Application::~Application()
	{
		Renderer::GUI::Shutdown();
		Renderer::Context::Shutdown();
		Camera::Shutdown();
		Platform::MainWindow::Shutdown();
	}

	void Application::Run()
	{
		while (!Platform::MainWindow::GetRef().ShouldQuit())
		{
			mTimestep->OnTick();
		}
	}

	void Application::OnSyncUpdate()
	{
		// Todo: Implement interpolation beetween previous frame data and current frame data
		// a frame data will be a class/struct that holds info about the physics, like position on world
	}

	void Application::OnAsyncUpdate()
	{
		double ts = mTimestep->GetTimestep();

		Platform::MainWindow::GetRef().OnUpdate();
		Camera::GetRef().OnUpdate(ts);
		Renderer::GUI::GetRef().OnUpdate();
		Renderer::Context::GetRef().OnUpdate();
	}

	void Application::OnEvent(Shared<Platform::EventBase> event)
	{
		Camera::GetRef().OnEvent(event);
		Renderer::GUI::GetRef().OnEvent(event);
	}
}