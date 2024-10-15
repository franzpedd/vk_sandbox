#pragma once

#include <Renderer/GUI/Widget.h>

namespace Cosmos::Editor
{
	class ImDemo : public Renderer::Widget
	{
	public:

		// constructor
		ImDemo();

		// destructor
		virtual ~ImDemo() = default;

	public:

		// user interface updating
		virtual void OnUpdate() override;

	public:

		// sets on/off the debug window
		inline void SetOpened(bool value) { mOpened = value; }

		// returns if the debug window is opened
		inline bool IsOpened() { return mOpened; }

	private:

		bool mOpened = false;
	};
}