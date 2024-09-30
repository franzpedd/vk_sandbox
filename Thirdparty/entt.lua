project "entt"
    location "entt"
    kind "None"
    language "C++"
    cppdialect "C++11"
    staticruntime "On"

    targetdir(paths.Binary)
    objdir(paths.Temp)

    files
    {
        "entt/entt.hpp"
    }

    includedirs
    {
        "entt"
    }

    filter "configurations:Debug"
        warnings "Off"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        optimize "Full"

    