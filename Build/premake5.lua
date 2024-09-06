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
paths = {};
paths["Temp"]       = "%{wks.location}/Bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}";
paths["Binary"]     = "%{wks.location}/Bin/temp/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}";
-- projects
paths["Workspace"]  = "../";
paths["Common"]     = "../Common";
paths["Platform"]   = "../Platform";
paths["Renderer"]   = "../Renderer";
paths["Engine"]     = "../Engine";
paths["Editor"]     = "../Editor";
-- dependencies
paths["GLM"]        = "../Thirdparty/glm";
paths["Volk"]       = "../Thirdparty/volk";
paths["VMA"]        = "../Thirdparty/vma/include";
paths["STB"]        = "../Thirdparty/stb"
paths["ImGui"]      = "../Thirdparty/imgui";

-- projects
group "Thirdparty"
    include "GLM"
    include "ImGui"
group ""

group "Libraries"
    include "Common.lua"
    include "Renderer.lua";
    include "Platform.lua";
    include "Engine.lua";
group ""

include "Editor.lua";