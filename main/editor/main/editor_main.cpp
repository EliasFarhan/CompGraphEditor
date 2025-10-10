#include <argh.h>
#include <SDL3/SDL_main.h>
#include "engine/engine.h"
#include "engine/filesystem.h"
#include "editor.h"
#include "py_interface.h"
#include "gl/engine.h"
#include "utils/log.h"

int main([[maybe_unused]] int argc, char** argv)
{
    argh::parser cmdl(argv);



    core::EnableLogRecording();
    gl::Engine engine;
    int major = 0, minor = 0;
    if (cmdl({ "-M", "--major" }) >> major && cmdl({ "-m", "--minor" }) >> minor)
    {
        engine.SetVersion(major, minor, cmdl[{ "-es", "--es" }]);
    }
    engine.SetWindowName("Neko3d Editor");

    core::PyManager pyManager;

    editor::Editor editor;
    engine.RegisterSystem(&editor);
    engine.RegisterOnGuiInterface(&editor);
    engine.RegisterEventObserver(&editor);

    engine.Run();

    return 0;
}