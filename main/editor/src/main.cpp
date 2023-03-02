#include <SDL_main.h>
#include "engine/engine.h"
#include "engine/filesystem.h"
#include "editor.h"
#include "gl/engine.h"
#include "utils/log.h"

int main(int argc, char** argv)
{
    core::EnableLogRecording();
    core::DefaultFilesystem filesystem;
    core::FilesystemLocator::provide(&filesystem);
    gl::Engine engine;
    engine.SetWindowName("GPR5300 Editor");

    gpr5300::Editor editor;
    engine.RegisterSystem(&editor);
    engine.RegisterImGuiDrawInterface(&editor);
    engine.RegisterEventObserver(&editor);

    engine.Run();

    return 0;
}