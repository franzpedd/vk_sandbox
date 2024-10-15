#pragma once

#include <Platform/Core/PlatformDetection.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_IMPL_VULKAN_USE_VOLK
#include <imgui.h>
#include <imgui_internal.h>

#if defined(PLATFORM_WINDOWS)
#pragma warning(push)
#pragma warning(disable : 26495 6263 6001 6255)
#endif

#include <ImGuizmo.h>

#if defined(PLATFORM_WINDOWS)
#pragma warning(pop)
#endif