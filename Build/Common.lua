project "Common"
    location "../Common"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "On" -- affects only windows
    linkgroups "On" -- affects only linux

    targetdir(paths.Binary)
    objdir(paths.Temp)

    files
    {
        "%{paths.Common}/**.h",
        "%{paths.Common}/**.cpp"
    }
    
    includedirs
    {
        "%{paths.Workspace}",
        "%{paths.Common}",
        --
        "%{paths.glm}"
    }

    links
    {
        
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