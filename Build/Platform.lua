project "Platform"
    location "../Platform"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "On" -- affects only windows
    linkgroups "On" -- affects only linux
    
    targetdir(paths.Binary)
    objdir(paths.Temp)

    files
    {
        "%{paths.Platform}/**.h",
        "%{paths.Platform}/**.cpp"
    }
    
    includedirs
    {
        "%{paths.Workspace}",
        "%{paths.Common}",
        "%{paths.Platform}",
        --
        "%{paths.glfw}",
        "%{paths.glm}",
        "%{paths.spdlog}",
        "%{paths.stb}"
    }

    links
    {
        "glfw"
    }

    defines
    {
        "RENDERER_VULKAN"
    }

    if os.host() == "windows" then
        defines
        {
            "_CRT_SECURE_NO_WARNINGS"
        }
    end
    
    filter "configurations:Debug"
        defines { "COMMON_DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "COMMON_RELEASE" }
        runtime "Release"
        optimize "On"