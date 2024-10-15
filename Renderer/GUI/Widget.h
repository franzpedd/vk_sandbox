#pragma once

#include <Common/Util/Memory.h>

// forward declarations
namespace Cosmos::Platform { class EventBase; }

namespace Cosmos::Renderer
{
	class Widget
	{
	public:

		// constructor
		Widget(const char* name) : mName(name) {};

		// destructor
		virtual ~Widget() = default;

		// returns it's name
		inline const char* GetName() { return mName; }

	public:

		// user interface drawing
		virtual void OnUpdate() {};

		// renderer drawing
		virtual void OnRender() {};

		// called when an event happens
		virtual void OnEvent(Shared<Platform::EventBase> event) {};

	private:

		const char* mName;
	};
}