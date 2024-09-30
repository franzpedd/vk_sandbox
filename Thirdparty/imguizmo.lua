project "imguizmo"
    location "imguizmo"
    kind "None"
    language "C++"
    cppdialect "C++11"
    staticruntime "On"

    targetdir(paths.Binary)
    objdir(paths.Temp)

    files
    {
        "imguizmo/ImGuizmo.h",
        "imguizmo/ImGuizmo.cpp"
    }

    includedirs
    {
        "imguizmo"
    }

    filter "configurations:Debug"
        warnings "Off"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        optimize "Full"

    