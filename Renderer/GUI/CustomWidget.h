#pragma once

namespace Cosmos::Renderer
{
	class CustomWidget
	{
	public:

		// slides into on/off position
		static bool CheckboxSliderEx(const char* label, bool* v);
	};
}