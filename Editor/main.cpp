#include "Core/Application.h"

#include <Common/Debug/Profiler.h>
#include <Engine/Core/Project.h>

#include <iostream>

int main(int argc, char* argv[])
{
    // setup initial settings
    auto settings = Cosmos::Engine::ProjectSettings::Read("Settings.ini");
    auto project = Cosmos::CreateShared<Cosmos::Engine::Project>(settings);

    // create the application
    PROFILER_BEGIN("Startup", "Profile-Startup.json");
    Cosmos::Editor::Application app(project);
    PROFILER_END();

    PROFILER_BEGIN("Runtime", "Profile-Runtime.json");
    app.Run();
    PROFILER_END();

    return 0;
}