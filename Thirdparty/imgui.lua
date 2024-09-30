project "imgui"
    location "imgui"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "On"

    targetdir(paths.Binary)
    objdir(paths.Temp)

    files
    {
        "imgui/imconfig.h",
        "imgui/imgui_demo.cpp",
        "imgui/imgui_draw.cpp",
        "imgui/imgui_internal.h",
        "imgui/imgui_tables.cpp",
        "imgui/imgui_widgets.cpp",
        "imgui/imgui.h",
        "imgui/imgui.cpp",
        "imgui/imstb_rectpack.h",
        "imgui/imstb_textedit.h",
        "imgui/imstb_truetype.h",

        --"imgui/backends/imgui_impl_glfw.cpp",
        "imgui/backends/imgui_impl_glfw.h",
        --"imgui/backends/imgui_impl_vulkan.cpp",
        "imgui/backends/imgui_impl_vulkan.h"
    }

    includedirs
    {
        "imgui",
        "glfw/include",
        "Vulkan",
        "vma",
        "volk"
    }

    defines
    {
        "IMGUI_IMPL_VULKAN_USE_VOLK", "IMGUI_IMPL_VULKAN_NO_PROTOTYPES"
    }

    disablewarnings
    { 
       "26819"
    }

    filter "configurations:Debug"
        warnings "Off"
        runtime "Debug"
        symbols "On"
    
    filter "configurations:Release"
        warnings "Default"
        runtime "Release"
        optimize "On"

    