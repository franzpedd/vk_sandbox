#pragma once

#include <Common/Util/Memory.h>

// forward declarations
namespace Cosmos::Engine { class Timestep; }
namespace Cosmos::Platform { class EventBase; }

namespace Cosmos::Engine
{
	class Application
	{
	public:

		// constructor
		Application();

		// destructor
		virtual ~Application();

		// returns a reference to the timestep
		inline Unique<Timestep>& GetTimestepRef() { return mTimestep; }

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
		
		Unique<Timestep> mTimestep;
	};
}