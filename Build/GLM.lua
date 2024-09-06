project "GLM"
    location "../Thirdparty/glm"
    kind "None"
    language "C++"
    cppdialect "C++11"
    staticruntime "On"

    targetdir(paths.Binary)
    objdir(paths.Temp)

    files
    {
        "../Thirdparty/glm/glm/**.hpp"
    }

    includedirs
    {
        "%{paths.GLM}"
    }

    filter "configurations:Debug"
        warnings "Off"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        optimize "Full"

    