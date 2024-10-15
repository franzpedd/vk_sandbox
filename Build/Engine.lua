project "Engine"
    location "../Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "On" -- affects only windows
    linkgroups "On" -- affects only linux

    targetdir(paths.Binary)
    objdir(paths.Temp)

    files
    {
        "%{paths.Engine}/**.h",
        "%{paths.Engine}/**.cpp"
    }

    includedirs
    {
        "%{paths.Workspace}",
        "%{paths.Engine}",
        --
        "%{paths.glm}",
        "%{paths.entt}",
        --
        "%{paths.Common}",
        "%{paths.Renderer}",
        "%{paths.Platform}"
    }

    defines
    {
        "RENDERER_VULKAN" -- allows the access to vulkan code on Renderer project
    }

    links
    {
        "Common",
        "Renderer",
        "Platform",
    }
    
    filter "configurations:Debug"
        defines { "ENGINE_DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "ENGINE_RELEASE" }
        runtime "Release"
        optimize "On"
    