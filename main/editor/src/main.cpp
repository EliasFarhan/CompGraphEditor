#include <SDL_main.h>
#include "engine/engine.h"
#include "engine/filesystem.h"

int main(int argc, char** argv)
{
    gpr5300::DefaultFilesystem filesystem;
    gpr5300::FilesystemLocator::provide(&filesystem);
    gpr5300::Engine engine;

    gpr5300::SceneEditor editor;
    engine.RegisterSystem(&editor);
    engine.RegisterImGuiDrawInterface(&editor);

    engine.Run();

    return 0;
}