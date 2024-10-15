#pragma once

#include <Common/Util/Memory.h>

// forward declarations
namespace Cosmos::Engine { class Project; }
namespace Cosmos::Engine { class Scene; }
namespace Cosmos::Engine { class Timestep; }
namespace Cosmos::Platform { class EventBase; }

namespace Cosmos::Engine
{
	class Application
	{
	public:

		// constructor
		Application(Shared<Project> project);

		// destructor
		virtual ~Application();

		// returns a reference to the timestep
		inline Unique<Timestep>& GetTimestepRef() { return mTimestep; }

		// returns a reference to the project
		inline Shared<Project>& GetProjectRef() { return mProject; }

		// returns the current scene
		inline Scene* GetCurrentScene() { return mCurrentScene; }

	public:

		// starts the main loop
		void Run();

		// This function is called at a fixed rate (depends on Timestep)
		void OnSyncUpdate();

		// This function called at an unfixed rate
		void OnAsyncUpdate();

		// called when an event happens
		void OnEvent(Shared<Platform::EventBase> event);

	private:

		Shared<Project> mProject;
		Unique<Timestep> mTimestep;
		Scene* mCurrentScene;
	};
}