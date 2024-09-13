project "Engine"
    location "../Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "On"

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
        "%{paths.GLM}",
        "%{paths.Entt}"
    }

    defines
    {
        "RENDERER_VULKAN" -- allows the access to vulkan code on Renderer project
    }
    
    filter "configurations:Debug"
        defines { "ENGINE_DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "ENGINE_RELEASE" }
        runtime "Release"
        optimize "On"
    