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
		Widget(const char* name, bool visible = false) : mName(name), mVisible(visible) {};

		// destructor
		virtual ~Widget() = default;

		// returns it's name
		inline const char* GetName() { return mName; }

		// returns if the widget is visible/displaying
		inline bool GetVisibility() { return mVisible; }

		// sets the widget visibility
		inline void SetVisibility(bool value) { mVisible = value; }

	public:

		// user interface drawing
		virtual void OnUpdate() {};

		// renderer drawing
		virtual void OnRender() {};

		// called when an event happens
		virtual void OnEvent(Shared<Platform::EventBase> event) {};

	protected:

		const char* mName = nullptr;
		bool mVisible = false;
	};
}