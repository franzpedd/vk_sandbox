#include "IGUI.h"

#include "GUI/Widget.h"
#include "Vulkan/GUI.h"
#include "Wrapper/imgui.h"
#include <Common/Debug/Logger.h>

namespace Cosmos::Renderer
{
	#ifdef RENDERER_VULKAN
	static Vulkan::GUI* s_Instance = nullptr;
	#else
	#error "Unsupported Renderer";
	#endif

	void IGUI::Initialize()
	{
		if (s_Instance) {
			COSMOS_LOG(Logger::Warn, "Attempting to initialize GUI when it's already initialized\n");
			return;
		}

		s_Instance = new Vulkan::GUI();
	}

	void IGUI::Shutdown()
	{
		delete s_Instance;
		s_Instance = nullptr;
	}

	IGUI* IGUI::GetRef()
	{
		if (!s_Instance) {
			COSMOS_LOG(Logger::Error, "The GUI has not been initialized\n");
		}

		return s_Instance;
	}

	void IGUI::ToggleCursor(bool hide)
	{
		ImGuiIO& io = ImGui::GetIO();

		if (hide) {
			io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
			io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
			return;
		}

		io.ConfigFlags ^= ImGuiConfigFlags_NoMouse;
	}

	void IGUI::AddWidget(Widget* widget)
	{
		mWidgets.Push(widget);
	}

	Widget* IGUI::FindWidgetByName(const char* name)
	{
		Widget* found = nullptr;

		for (auto& widget : mWidgets.GetElementsRef()) {
			if (widget->GetName() == name) {
				found = widget;
				break;
			}
		}

		return found;
	}
}