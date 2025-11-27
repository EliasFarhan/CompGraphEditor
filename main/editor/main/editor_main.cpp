#include <argh.h>
#include <SDL3/SDL_main.h>
#include "engine/engine.h"
#include "engine/filesystem.h"
#include "editor.h"
#include "py_interface.h"
#include "utils/log.h"
#include "novus/engine.h"

int main([[maybe_unused]] int argc, char** argv)
{
    argh::parser cmdl(argv);



    core::EnableLogRecording();
    novus::Engine engine;
    engine.SetWindowName("Neko3d Editor");

    core::PyManager pyManager;

    novus::editor::Editor editor;
    engine.RegisterSystem(&editor);
    engine.RegisterOnGuiInterface(&editor);
    engine.RegisterEventObserver(&editor);

    engine.Run();

    return 0;
}