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
        "%{paths.Editor}",
        --
        "%{paths.Common}",
        "%{paths.Platform}",
        "%{paths.Renderer}", 
        "%{paths.Engine}", 
        --
        "%{paths.GLM}",
        "%{paths.Volk}",
        "%{paths.VMA}",
        "%{paths.ImGui}",
        "%{paths.Entt}"
    }

    defines
    {
        "RENDERER_VULKAN"
    }

    links
    {
        "ImGui",
        "Common",
        "Renderer",
        "Platform",
        "Engine"
    }

    -- windows only
    if os.host() == "windows" then
        defines
        {
            "_CRT_SECURE_NO_WARNINGS"
        }

        includedirs
        {
            os.getenv("VULKAN_SDK") .. "/Include" -- vulkan
        }
    end

    -- linux only
    if os.host() == "linux" then
        links
        {
            "SDL2",
            "vulkan",
            "shaderc_shared"
        }
    end

    filter "configurations:Debug"
        defines { "EDITOR_DEBUG"}
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "EDITOR_RELEASE" }
        runtime "Release"
        optimize "On"