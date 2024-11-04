#pragma once

#include <Platform/Core/PlatformDetection.h>

#if defined(PLATFORM_WINDOWS)
#pragma warning(push)
#pragma warning(disable : 26495)
#endif

//#define TINYGLTF_NO_INCLUDE_STB_IMAGE 
//#define TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE
#define TINYGLTF_USE_RAPIDJSON
#include <tiny_gltf.h>

#if defined(PLATFORM_WINDOWS)
#pragma warning(pop)
#endif