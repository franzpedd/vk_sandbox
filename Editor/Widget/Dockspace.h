#pragma once

#include <Renderer/GUI/Widget.h>

namespace Cosmos::Editor
{
	class Dockspace : public Renderer::Widget
	{
	public:

		// constructor
		Dockspace();

		// destructor
		virtual ~Dockspace() = default;

	public:

		// updates the ui element
		virtual void OnUpdate() override;
	};
}