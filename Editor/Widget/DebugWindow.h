#pragma once

#include <Renderer/GUI/Widget.h>

namespace Cosmos::Editor { class Application; }

namespace Cosmos::Editor
{
	class DebugWindow : public Renderer::Widget
	{
	public:

		// constructor
		DebugWindow(Application* application);

		// destructor
		virtual ~DebugWindow() = default;

	public:

		// user interface updating
		virtual void OnUpdate() override;

	public:

		// sets on/off the debug window
		inline void SetOpened(bool value) { mOpened = value; }

		// returns if the debug window is opened
		inline bool IsOpened() { return mOpened; }

	private:
		
		Application* mApplication = nullptr;
		bool mOpened = true;
	};
}