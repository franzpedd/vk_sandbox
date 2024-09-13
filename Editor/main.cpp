#include "Core/Application.h"

#include <Engine/Core/Project.h>

#include <iostream>

int main(int argc, char* argv)
{
    // setup initial settings
    auto settings = Cosmos::Engine::ProjectSettings::Read("Settings.ini");
    auto project = Cosmos::CreateShared<Cosmos::Engine::Project>(settings);

    // create the application
    Cosmos::Editor::Application app(project);
    app.Run();

    return 0;
}