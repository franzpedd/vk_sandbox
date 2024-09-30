project "vma"
    location "vma"
    kind "None"
    language "C++"
    cppdialect "C++11"
    staticruntime "On"

    targetdir(paths.Binary)
    objdir(paths.Temp)

    files
    {
        "vma/vk_mem_alloc.h"
    }

    includedirs
    {
        "vma"
    }

    filter "configurations:Debug"
        warnings "Off"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        optimize "Full"