project "ImGui"
    location "../Thirdparty/imgui"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "On"

    targetdir(paths.Binary)
    objdir(paths.Temp)

    files
    {
        "../Thirdparty/imgui/imconfig.h",
        "../Thirdparty/imgui/imgui_demo.cpp",
        "../Thirdparty/imgui/imgui_draw.cpp",
        "../Thirdparty/imgui/imgui_internal.h",
        "../Thirdparty/imgui/imgui_tables.cpp",
        "../Thirdparty/imgui/imgui_widgets.cpp",
        "../Thirdparty/imgui/imgui.h",
        "../Thirdparty/imgui/imgui.cpp",
        "../Thirdparty/imgui/imstb_rectpack.h",
        "../Thirdparty/imgui/imstb_textedit.h",
        "../Thirdparty/imgui/imstb_truetype.h"
    }

    includedirs
    {
        "../Thirdparty/imgui"
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

    