project "Game"
    location "../Game"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "On" -- affects only windows
    linkgroups "On" -- affects only linux

    targetdir(paths.Binary)
    objdir(paths.Temp)

    files
    {
        "%{paths.Game}/**.h",
        "%{paths.Game}/**.cpp"
    }
    
    includedirs
    {
        "%{paths.Workspace}",
        "%{paths.Vulkan}",
        "%{paths.Game}",
        --
        "%{paths.entt}",
        "%{paths.glfw}",
        "%{paths.glm}",
        "%{paths.imgui}",
        "%{paths.imguizmo}",
        "%{paths.spdlog}",
        "%{paths.stb}",
        "%{paths.steamsdk}",
        "%{paths.rapidjson}",
        "%{paths.tinygltf}",
        "%{paths.vma}",
        "%{paths.volk}",
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
        "glfw",
        "imgui",
        "Common",
        "Platform",
        "Renderer",
        "Engine"
    }

    if os.host() == "windows" then
        disablewarnings { "26439" }
        defines { "_CRT_SECURE_NO_WARNINGS" }
        links 
        {
            os.getenv("VULKAN_SDK") .. "/Lib/shaderc_shared.lib",
            "%{paths.steamsdk}/bin/windows/steam_api64.lib",
            "%{paths.steamsdk}/bin/windows/sdkencryptedappticket64.lib",
        }
        
        prebuildcommands 
        {
            "{COPYFILE} %{paths.steamsdk}/steam_appid.txt " .. paths.Binary;
            "{COPYFILE} %{paths.steamsdk}/bin/windows/steam_api64.dll " .. paths.Binary;
            "{COPYFILE} %{paths.steamsdk}/bin/windows/sdkencryptedappticket64.dll " .. paths.Binary;
        }

        --filter { "configurations:Debug" }
            --prebuildcommands { "{COPYFILE} %{paths.networking}/bin_windows/Debug " .. paths.Binary; }
            --links { "%{paths.networking}/bin_windows/Debug/GameNetworkingSockets.lib" }

        --filter { "configurations:Release" }
            --prebuildcommands { "{COPYFILE} %{paths.networking}/bin_windows/Release " .. paths.Binary; }
            --links { "%{paths.networking}/bin_windows/Release/GameNetworkingSockets.lib" }
    end

    if os.host() == "linux" then
        links  { "X11", "shaderc_shared" }
    end

    filter "configurations:Debug"
        defines { "EDITOR_DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "EDITOR_RELEASE" }
        runtime "Release"
        optimize "On"