project "Platform"
    location "../Platform"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "On"

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
        "%{paths.Platform}",
        --
        "%{paths.GLM}"
    }

    defines
    {
        "RENDERER_VULKAN" -- neccessary to wrap the include of sdl_vulkan
    }

    -- windows only
    if os.host() == "windows" then
        staticruntime "On"

        includedirs
        {
            "../Thirdparty/sdl2/sdl2/include",      -- sdl
            "../Thirdparty/sdl2/sdl2image/include"  -- sdlimage
        }

        prebuildcommands
        {
            "{COPYFILE} ../Thirdparty/sdl2/sdl2/lib/x64/SDL2.dll " .. paths.Binary;
            "{COPYFILE} ../Thirdparty/sdl2/sdl2image/lib/x64/SDL2_image.dll " .. paths.Binary;
        }

        links
        {
            "../Thirdparty/sdl2/sdl2/lib/x64/SDL2.lib",             -- sdl2
            "../Thirdparty/sdl2/sdl2/lib/x64/SDL2main.lib",         -- sdl2 entrypoint
            "../Thirdparty/sdl2/sdl2image/lib/x64/SDL2_image.lib"   -- sdl2 image
        }

        defines
        {
            "_CRT_SECURE_NO_WARNINGS"
        }

        linkoptions
        {
            "/ignore:4006" -- sdl includes stuff already defined on shaderc
        }
    end

    if os.host() == "linux" then
        links
        {
            "SDL2",
            "SDL2_image"
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