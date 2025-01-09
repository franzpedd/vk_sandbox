#pragma once

#include <Common/Util/Memory.h>

// forwar ddeclarations
namespace Cosmos::Platform { class EventBase; }

namespace Cosmos::Engine
{
	class Extension
	{
	public:

		// constructor
		Extension() = default;

		// destructor
		virtual ~Extension() = default;

	public:

		// called for logic updating/ui updating called once per tick
		virtual void OnUpdate(float timestep) {};

		// called for drawing on the scene
		virtual void OnRender() {};

		// called when input events happens
		virtual void OnEvent(Shared<Platform::EventBase> event) {};
	};
}