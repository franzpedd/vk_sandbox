#include "ImDemo.h"

#include <Wrapper/imgui.h>

namespace Cosmos::Editor
{
	ImDemo::ImDemo()
		: Widget("ImDemo")
	{
	}

	void ImDemo::OnUpdate()
	{
		if (mOpened) {
			ImGui::ShowDemoWindow();
		}
	}
}