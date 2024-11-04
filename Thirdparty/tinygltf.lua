project "tinygltf"
    location "tinygltf"
    kind "None"
    language "C++"
    cppdialect "C++11"
    staticruntime "On"

    targetdir(paths.Binary)
    objdir(paths.Temp)

    files
    {
        "tinygltf/tiny_gltf.h",
        "tinygltf/rapidjson/**.h"
    }

    includedirs
    {
        "tinygltf"
    }

    defines
    {
        "TINYGLTF_USE_RAPIDJSON"
    }

    if os.host() == "windows" then
        disablewarnings { "26495" }
    end

    filter "configurations:Debug"
        warnings "Off"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        optimize "Full"

    