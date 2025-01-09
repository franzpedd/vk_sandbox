#include "vulkan.h"

#if defined RENDERER_VULKAN

//////////////////////////////////////////////////////////////////////////////////////////////////////
// volk
#define VOLK_IMPLEMENTATION
#include <volk.h>
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////
// vulkan memory allocator
#if defined(PLATFORM_WINDOWS)
#pragma warning(push)
#pragma warning(disable : 26495 6386 6387 26110 26813)
#endif

#define VMA_VULKAN_VERSION 1002000 // vulkan memory allocator version is 1.2.0, same as in the instance
#define VMA_STATIC_VULKAN_FUNCTIONS 0 // not statically linking to vulkan 
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0 // not dynamically linking to vulkan
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#if defined(PLATFORM_WINDOWS)
#pragma warning(pop)
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////
// stb
#if defined(PLATFORM_WINDOWS)
#pragma warning(push)
#pragma warning(disable : 6272 6262 26827 26819 4996)
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#if defined(PLATFORM_WINDOWS)
#pragma warning(pop)
#endif