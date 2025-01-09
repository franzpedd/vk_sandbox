#pragma once

#include <Common/Math/Math.h>
#include <string>

namespace Cosmos::Renderer
{
	class CustomWidget
	{
	public:

		// slides into on/off position
		static bool CheckboxSliderEx(const char* label, bool* v);

		// custom checkbox with color on the selected mark
		static bool Checkbox(const char* label, bool* v);

		// custom vector-3 controls
		static bool Vector3Control(const char* label, glm::vec3& values);

		// adds a centered text on the window
		static void TextCentered(std::string text);
	};
}