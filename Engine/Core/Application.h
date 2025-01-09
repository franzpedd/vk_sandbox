#pragma once

#include <Common/Util/Library.h>
#include <Common/Util/Memory.h>

// forward declarations
namespace Cosmos::Engine { class Extension; }
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

		// returns a reference to the extensions
		inline Library<Extension*>& GetExtensionsRef() { return mExtensions; }

	public:

		// starts the main loop
		void Run();

		// called when an event happens
		void OnEvent(Shared<Platform::EventBase> event);

		// called for drawing into the scene, this is used by extensions, this must match Renderer::IContext::Stage
		void OnRender(uint32_t stage); 

	protected:

		Shared<Project> mProject;
		Unique<Timestep> mTimestep;
		Scene* mCurrentScene;
		Library<Extension*> mExtensions;
	};
}