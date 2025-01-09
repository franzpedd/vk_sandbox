#include "Utils.h"

#include <GLFW/glfw3.h>

namespace Cosmos::Platform
{
	uint64_t GameTicks()
	{
		return (uint64_t)glfwGetTime();
	}

	uint64_t GetGameTimeInSeconds()
	{
		return GameTicks() / 1000;
	}
}