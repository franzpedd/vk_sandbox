# About
Currently it's only a Vulkan Renderer on top of an Editor application (It uses C++17). You can check it's development screenshots under the [Documentation](Documentation) folder. If you're looking for a C++ Framework with a Vulkan Renderer this may be for you, if you're looking to create a game with it I would advise waiting for further development. If you'd like to create a suggestion or found any bug, feel free to create an issue.

# Build
Right now the project is only tested on Windows and Linux;

The Vulkan SDK is only required because of shader compilation, wich happens on runtime, this should change in the future in order to facilitate publishing an application. (You wouldn't want to enforce the installation of the Vulkan SDK on a client).

All dependencies are located on [Thirdparty](Thirdparty) folder, this is in order facilitate the building process (all licenses are present), they could be present in the form of submodules however it's an unnecessary pain to work with them in the beginning of the project, therefore they are here. 

I also dislike distributing binaries on any form whatsoever, therefore source code is the only form of code in this repo. I've choosen to use premake for the project generation (CMake would be a better option but I'm stubborn, sorry for that); Nevermind, the process should be simple enough.

The building process:
* Install [Vulkan SDK](https://vulkan.lunarg.com/) (shaderc will be installed alongside it, all other vulkan stuff are dynamically loaded);
* Clone this repo;
* Download [Premake5](https://premake.github.io/) and place it under [Build](Build) folder (or setup environment variables to it);
* Use the desired command to generate the desired solution ([See Here](https://premake.github.io/docs/Using-Premake));

# Near Roadmap
* Add Lights Emitters and Implement Cluster Shading;
* Implement PBR on meshes;
* Add Native Scripting System;

# Credits
The dependencies this project uses at this momment are: 
[EnTT](https://github.com/skypjack/entt), 
[GLFW](https://github.com/glfw/glfw),
[GLM](https://glm.g-truc.net/),
[ImGui](https://github.com/ocornut/imgui/), 
[ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo), 
[RapidJSON](https://github.com/Tencent/rapidjson), 
[spdlog](https://github.com/gabime/spdlog), 
[STB](https://github.com/nothings/stb), 
[tinygltf](https://github.com/syoyo/tinygltf), 
[VMA](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
[Volk](https://github.com/zeux/volk);
It also uses [Premake](https://premake.github.io/) for solution generation, [Lucide](https://lucide.dev/) and [Font Awesome v4](https://fontawesome.com/v4) icons.

I'm not creating this out of nowhere and in fact, I'm learning as I develop it. This requires a bunch of resources, some of these resources were made by:
* Sascha Willems: Vulkan code and examples;
* Javidx9: Serialization/Deserialization; 
* The Cherno: Widgets Layering and ECS example;
* Planet Chili: 3D Programming Concepts;

If you're missing on this list, please create an issue.