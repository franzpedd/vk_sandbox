#include "Application.h"

#include "Project.h"
#include "Scene.h"
#include "Timestep.h"
#include "Entity/Camera.h"
#include <Common/Debug/Logger.h>
#include <Common/Debug/Profiler.h>
#include <Common/File/Filesystem.h>
#include <Platform/Core/MainWindow.h>
#include <Renderer/Core/Context.h>
#include <Renderer/GUI/GUI.h>

namespace Cosmos::Engine
{
	Application::Application(Shared<Project> project)
		: mProject(project)
	{
		PROFILER_FUNCTION();

		auto& settings = mProject->GetSettingsRef();
		SetAssetsDir(settings.datapath);

		Platform::MainWindow::Initialize(this, settings.gamename.c_str(), settings.width, settings.height, settings.fullscreen);
		Camera::Initialize(Platform::MainWindow::GetRef().GetAspectRatio());
		Renderer::Context::Initialize(this);
		Renderer::GUI::Initialize();
		mCurrentScene = new Scene(settings.initialscene);
		mTimestep = CreateUnique<Timestep>();
	}

	Application::~Application()
	{
		PROFILER_FUNCTION();

		delete mCurrentScene;
		Renderer::GUI::Shutdown();
		Renderer::Context::Shutdown();
		Camera::Shutdown();
		Platform::MainWindow::Shutdown();
	}

	void Application::Run()
	{
		PROFILER_FUNCTION();

		Platform::MainWindow& window = Platform::MainWindow::GetRef();
		Camera& camera = Camera::GetRef();
		Renderer::GUI& gui = Renderer::GUI::GetRef();
		Renderer::Context& renderer = Renderer::Context::GetRef();

		while (!window.ShouldQuit())
		{
			PROFILER_SCOPE("MainLoop");

			// fps and deltatime timer begins
			mTimestep->StartFrame();
			float ts = mTimestep->GetTimestep();

			// frame logic
			window.OnUpdate();
			camera.OnUpdate(ts);
			mCurrentScene->OnUpdate(ts);
			gui.OnUpdate();
			renderer.OnUpdate();

			// fps and deltatime timer stops
			mTimestep->EndFrame();
		}
	}

	void Application::OnEvent(Shared<Platform::EventBase> event)
	{
		PROFILER_FUNCTION();

		Camera::GetRef().OnEvent(event);
		Renderer::GUI::GetRef().OnEvent(event);
	}
}