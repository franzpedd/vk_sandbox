project "spdlog"
    location "spdlog"
    kind "None"
    language "C++"
    cppdialect "C++11"
    staticruntime "On"

    targetdir(paths.Binary)
    objdir(paths.Temp)

    files
    {
        "spdlog/include/**.h"
    }

    includedirs
    {
        "spdlog",
        "%{paths.rapidjson}"
    }

    filter "configurations:Debug"
        warnings "Off"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        optimize "Full"

    