#pragma once

// global
//// uncoment to enables/disables profiling
//#define COSMOS_PROFILE

//// how long a path may be on chars count
#define COSMOS_PATH_MAX_SIZE 128

//// how long a short display name may be on chars count
#define COSMOS_DISPLAY_NAME_MAX_SIZE 32

// renderer
//// enables vulkan on the code, this is used for intellisense but will force the compilation with vulkan
#define RENDERER_VULKAN 1

//// how many frames are simultaneously rendered, to avoid frame tearing
#define CONCURENTLY_RENDERED_FRAMES 2

//// how many joints a mesh may have, this must be changed on shader as well
#define COSMOS_MESH_MAX_JOINTS 128u