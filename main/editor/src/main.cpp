#include <SDL_main.h>
#include "engine/engine.h"
#include "engine/filesystem.h"
#include "editor.h"
#include "gl/engine.h"
#include "utils/log.h"

int main(int argc, char** argv)
{
    gpr5300::EnableLogRecording();
    gpr5300::DefaultFilesystem filesystem;
    gpr5300::FilesystemLocator::provide(&filesystem);
    gpr5300::gl::Engine engine;
    engine.SetWindowName("GPR5300 Editor");

    gpr5300::Editor editor;
    engine.RegisterSystem(&editor);
    engine.RegisterImGuiDrawInterface(&editor);
    engine.RegisterEventObserver(&editor);

    engine.Run();

    return 0;
}