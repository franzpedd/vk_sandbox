project "stb"
    location "stb"
    kind "None"
    language "C++"
    cppdialect "C++11"
    staticruntime "On"

    targetdir(paths.Binary)
    objdir(paths.Temp)

    files
    {
        "stb/stb_image_write.h",
        "stb/stb_image.h"
    }

    includedirs
    {
        "stb"
    }

    filter "configurations:Debug"
        warnings "Off"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        optimize "Full"

    