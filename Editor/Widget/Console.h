#pragma once

#include <Renderer/GUI/Widget.h>

namespace Cosmos::Editor
{
	class Console : public Renderer::Widget
	{
	public:

		// constructor
		Console();

		// destructor
		virtual ~Console() = default;

	public:

		// called on loop update
		virtual void OnUpdate() override;

	public:

		// sets on/off the debug window
		inline void SetOpened(bool value) { mOpened = value; }

		// returns if the debug window is opened
		inline bool IsOpened() { return mOpened; }

	private:

		bool mOpened = true;
	};
}