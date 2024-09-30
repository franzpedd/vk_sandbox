-- global specification
workspace "Solution";
architecture "x86_64";
configurations { "Debug", "Release" };
startproject "Editor";

-- toolset used
if os.host() == "linux" then toolset("clang") end;
if os.host() == "windows" then toolset("msc") end;
if os.host() == "macosx" then toolset("clang") end;

-- paths used accross the scripts
---- general
paths = {};
paths["Temp"]       = "%{wks.location}/Bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}";
paths["Binary"]     = "%{wks.location}/Bin/temp/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}";
paths["Workspace"]  = "../";
paths["Vulkan"] = os.getenv("VULKAN_SDK") .. "/Include"
---- dependencies
paths["entt"]  = "../Thirdparty/entt";
paths["glfw"]  = "../Thirdparty/glfw/include";
paths["glm"]  = "../Thirdparty/glm";
paths["imgui"]  = "../Thirdparty/imgui";
paths["imguizmo"]  = "../Thirdparty/imguizmo";
paths["stb"]  = "../Thirdparty/stb";
paths["vma"]  = "../Thirdparty/vma";
paths["volk"]  = "../Thirdparty/volk";
---- libraries
paths["Common"] = "../Common";
paths["Platform"] = "../Platform";
paths["Renderer"] = "../Renderer";
paths["Engine"] = "../Engine";
---- applications
paths["Editor"]  = "../Editor";

-- project inclusion
---- dependencies
group "Thirdparty"
    include "../Thirdparty/entt.lua"
    include "../Thirdparty/glfw.lua"
    include "../Thirdparty/glm.lua"
    include "../Thirdparty/imgui.lua"
    include "../Thirdparty/imguizmo.lua"
    include "../Thirdparty/stb.lua"
    include "../Thirdparty/vma.lua"
    include "../Thirdparty/volk.lua"
group ""
---- libraries
group "Library"
    include "Common.lua";
    include "Platform.lua";
    include "Renderer.lua";
    include "Engine.lua"
group ""
---- applications
group "Application"
    include "Editor.lua";
group ""