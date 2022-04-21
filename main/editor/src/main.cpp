#include <SDL_main.h>
#include "engine/engine.h"
#include "engine/filesystem.h"
#include "scene_editor.h"

int main(int argc, char** argv)
{
    gpr5300::DefaultFilesystem filesystem;
    gpr5300::FilesystemLocator::provide(&filesystem);
    gpr5300::Engine engine;
    engine.SetWindowName("GPR5300 Editor");

    gpr5300::SceneEditor editor;
    engine.RegisterSystem(&editor);
    engine.RegisterImGuiDrawInterface(&editor);

    engine.Run();

    return 0;
}