#pragma once

#include <Common/Math/Math.h>

namespace Cosmos::Renderer
{
	class CustomWidget
	{
	public:

		// slides into on/off position
		static bool CheckboxSliderEx(const char* label, bool* v);

		// custom vector-3 controls
		static bool Vector3Control(const char* label, glm::vec3& values);
	};
}