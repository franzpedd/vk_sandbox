#include "Application.h"

#include "Project.h"
#include "Scene.h"
#include "Timestep.h"
#include "Entity/Camera.h"
#include <Common/File/Filesystem.h>
#include <Common/Debug/Logger.h>
#include <Platform/Core/MainWindow.h>
#include <Renderer/Core/Context.h>
#include <Renderer/GUI/GUI.h>

namespace Cosmos::Engine
{
	Application::Application(Shared<Project> project)
		: mProject(project)
	{
		auto& settings = mProject->GetSettingsRef();
		SetAssetsDir(settings.datapath);

		Platform::MainWindow::Initialize(this, settings.gamename.c_str(), settings.width, settings.height, settings.fullscreen);
		Camera::Initialize(Platform::MainWindow::GetRef().GetAspectRatio());
		Renderer::Context::Initialize(this);
		Renderer::GUI::Initialize();
		mCurrentScene = new Scene(settings.initialscene);
		mTimestep = CreateUnique<Timestep>(this);
	}

	Application::~Application()
	{
		delete mCurrentScene;
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
		double ts = mTimestep->GetTimestep();

		mCurrentScene->OnUpdate(ts);
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