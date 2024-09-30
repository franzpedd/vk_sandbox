project "glfw"
    location "glfw"
    kind "StaticLib"
    language "C"
    cdialect "C11"
    staticruntime "On"

    targetdir(paths.Binary)
    objdir(paths.Temp)

    files
    {
        "glfw/include/**.h",
        "glfw/src/context.c",
        "glfw/src/init.c",
        "glfw/src/input.c",
        "glfw/src/monitor.c",
        "glfw/src/platform.c",
        "glfw/src/vulkan.c",
        "glfw/src/window.c",
        
        "glfw/src/null_init.c",
        "glfw/src/null_joystick.c",
        "glfw/src/null_joystick.h",
        "glfw/src/null_monitor.c",
        "glfw/src/null_platform.h",
        "glfw/src/null_window.c"
    }

    includedirs
    {
        "glfw/include"
    }

    if os.host() == "windows" then
        files
        {
            "glfw/src/egl_context.c",
            "glfw/src/wgl_context.c",
            "glfw/src/osmesa_context.c",

            "glfw/src/win32_init.c",
            "glfw/src/win32_joystick.c",
            "glfw/src/win32_module.c",
            "glfw/src/win32_monitor.c",
            "glfw/src/win32_thread.c",
            "glfw/src/win32_time.c",
            "glfw/src/win32_window.c",
            
        }

        defines { "_GLFW_WIN32", "_CRT_SECURE_NO_WARNINGS" }
        disablewarnings { "4996", "5105" }
    end
    
    if os.host() == "linux" then
        files
        {
            "Gglfw/src/egl_context.c",
            "Gglfw/src/glx_context.c",

            "glfw/src/wl_init.c",
            "glfw/src/linux_joystick.c",
            "glfw/src/posix_module.c",
            "glfw/src/wl_monitor.c",
            "glfw/src/posix_thread.c",
            "glfw/src/posix_time.c",
            "glfw/src/wl_window.c"
        }

        defines { "_GLFW_X11" }
    end

    if os.host() == "macosx" then
    filter "system:macosx"
        files
        {
            "glfw/src/egl_context.c",
            "glfw/src/nsgl_context.m",

            "glfw/src/cocoa_init.m",
            "glfw/src/cocoa_joystick.m",
            "glfw/src/posix_module.c",
            "glfw/src/cocoa_monitor.m",
            "glfw/src/posix_thread.c",
            "glfw/src/posix_time.c",
            "glfw/src/cocoa_window.m"
        }

        defines { "_GLFW_COCOA" }
    end

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        optimize "Full"