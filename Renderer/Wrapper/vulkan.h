#pragma once
#if defined RENDERER_VULKAN

#include <Platform/Core/PlatformDetection.h>

// first include volk
#include <volk.h>

#if defined(PLATFORM_WINDOWS)
#pragma warning(push)
#pragma warning(disable : 26495 6255 6001)
#endif

//#define VMA_DEBUG_LOG 
#define VK_NO_PROTOTYPES
#define VMA_VULKAN_VERSION 1002000 // vulkan memory allocator version is 1.2.0, same as in the instance
#define VMA_STATIC_VULKAN_FUNCTIONS 0 // not statically linking to vulkan 
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0 // not dynamically linking to vulkan 
#include <vk_mem_alloc.h>

#if defined(PLATFORM_WINDOWS)
#pragma warning(pop)
#endif

#endif