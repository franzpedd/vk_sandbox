#include "Application.h"

#include "Extension.h"
#include "Project.h"
#include "Scene.h"
#include "Timestep.h"
#include "Entity/Camera.h"
#include <Common/Debug/Logger.h>
#include <Common/Debug/Profiler.h>
#include <Common/File/Filesystem.h>
#include <Platform/Core/MainWindow.h>
#include <Renderer/Core/IContext.h>
#include <Renderer/Core/IGUI.h>

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
		Renderer::IContext::Initialize(this);
		Renderer::IGUI::Initialize();
		mCurrentScene = new Scene(settings.initialscene);
		mTimestep = CreateUnique<Timestep>();
	}

	Application::~Application()
	{
		PROFILER_FUNCTION();

		// kill all extensions
		for (auto& extension : mExtensions.GetAllRefs()) {
			delete extension.second;
		}

		delete mCurrentScene;
		Renderer::IGUI::Shutdown();
		Renderer::IContext::Shutdown();
		Camera::Shutdown();
		Platform::MainWindow::Shutdown();
	}

	void Application::Run()
	{
		PROFILER_FUNCTION();

		Platform::MainWindow& window = Platform::MainWindow::GetRef();
		Camera& camera = Camera::GetRef();
		Renderer::IGUI* gui = Renderer::IGUI::GetRef();
		Renderer::IContext* renderer = Renderer::IContext::GetRef();

		while (!window.ShouldQuit())
		{
			PROFILER_SCOPE("MainLoop");

			// fps and deltatime timer begins
			mTimestep->StartFrame();
			float ts = mTimestep->GetTimestep();

			// update frame logic
			window.OnUpdate();
			camera.OnUpdate(ts);

			// update extensions
			for (auto& item : mExtensions.GetAllRefs()) {
				if (item.second != nullptr) {
					item.second->OnUpdate(ts);
				}
			}

			mCurrentScene->OnUpdate(ts);
			gui->OnUpdate();
			renderer->OnUpdate();

			// fps and deltatime timer stops
			mTimestep->EndFrame();
		}
	}

	void Application::OnEvent(Shared<Platform::EventBase> event)
	{
		PROFILER_FUNCTION();

		Camera::GetRef().OnEvent(event);
		Renderer::IContext::GetRef()->OnEvent(event);
		Renderer::IGUI::GetRef()->OnEvent(event);

		for (auto& item : mExtensions.GetAllRefs()) {
			if (item.second != nullptr) {
				item.second->OnEvent(event);
			}
		}
	}

	void Application::OnRender(uint32_t stage)
	{
		mCurrentScene->OnRender(stage);

		for (auto& item : mExtensions.GetAllRefs()) {
			if (item.second != nullptr) {
				item.second->OnRender();
			}
		}
	}
}