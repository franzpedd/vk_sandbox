project "Renderer"
    location "../Renderer"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "On" -- affects only windows
    linkgroups "On" -- affects only linux

    targetdir(paths.Binary)
    objdir(paths.Temp)

    files
    {
        "%{paths.Renderer}/**.h",
        "%{paths.Renderer}/**.cpp"
    }

    includedirs
    {
        "%{paths.Workspace}",
        "%{paths.Renderer}",
        "%{paths.Vulkan}",
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
        "%{paths.Common}"
    }

    defines
    {
        "RENDERER_VULKAN" -- includes vulkan stuff on the building process
    }

    links
    {
        "imgui",
        "glfw",
        "Common"
    }

    if os.host() == "windows" then
        links { os.getenv("VULKAN_SDK") .. "/Lib/shaderc_shared.dll" }
        disablewarnings { "26439" }
    end

    if os.host() == "linux" then
        links { "shaderc_shared" }
    end

    filter "configurations:Debug"
        defines { "PLATFORM_DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "PLATFORM_RELEASE" }
        runtime "Release"
        optimize "On"