project "Editor"
    location "../Editor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "On"

    targetdir(paths.Binary)
    objdir(paths.Temp)

    files
    {
        "%{paths.Editor}/**.h",
        "%{paths.Editor}/**.cpp"
    }
    
    includedirs
    {
        "%{paths.Workspace}",
        "%{paths.Vulkan}",
        "%{paths.Editor}",
        --
        "%{paths.entt}",
        "%{paths.glfw}",
        "%{paths.glm}",
        "%{paths.imgui}",
        "%{paths.imguizmo}",
        "%{paths.stb}",
        "%{paths.volk}",
        "%{paths.vma}",
        --
        "%{paths.Common}",
        "%{paths.Platform}",
        "%{paths.Renderer}",
        "%{paths.Engine}"
    }

    defines
    {
        "RENDERER_VULKAN"
    }

    links
    {
        "Common",
        "Platform",
        "Renderer",
        "Engine"
    }

    if os.host() == "windows" then
        defines
        {
            "_CRT_SECURE_NO_WARNINGS"
        }
    end

    filter "configurations:Debug"
        defines { "EDITOR_DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "EDITOR_RELEASE" }
        runtime "Release"
        optimize "On"