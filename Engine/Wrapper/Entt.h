#pragma once

#include <Platform/Core/PlatformDetection.h>

#if defined(PLATFORM_WINDOWS)
#pragma warning(push)
#pragma warning(disable : 26827)
#endif

#include <entt.hpp>

#if defined(PLATFORM_WINDOWS)
#pragma warning(pop)
#endif