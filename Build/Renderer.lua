project "Renderer"
    location "../Renderer"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"

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
        --
        "%{paths.GLM}",
        "%{paths.Volk}",
        "%{paths.VMA}",
        "%{paths.STB}",
        "%{paths.ImGui}"
    }

    defines
    {
        "RENDERER_VULKAN" -- includes vulkan stuff on the building process
    }

    links
    {
        "Common",
        "ImGui"
    }

    -- windows only
    if os.host() == "windows" then
        staticruntime "On"

        includedirs
        {
            os.getenv("VULKAN_SDK") .. "/Include",  -- vulkan
            "../Thirdparty/sdl2/sdl2/include"       -- sdl path, for ui 
        }

        links
        {
            os.getenv("VULKAN_SDK") .. "/Lib/shaderc_shared.lib",   -- shaderc
        }

        defines
        {
            "_CRT_SECURE_NO_WARNINGS"
        }

        disablewarnings
        { 
           "26819"
        }

        linkoptions
        {
            "/ignore:4006" -- sdl includes stuff already defined on shaderc
        }
    end

    if os.host() == "linux" then
        links
        {
            "shaderc_shared"
        }
    end

    filter "configurations:Debug"
        defines { "PLATFORM_DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "PLATFORM_RELEASE" }
        runtime "Release"
        optimize "On"