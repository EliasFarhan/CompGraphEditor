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
    engine.SetWindowName("CompGraph Editor");

    editor::Editor editor;
    engine.RegisterSystem(&editor);
    engine.RegisterOnGuiInterface(&editor);
    engine.RegisterEventObserver(&editor);

    engine.Run();

    return 0;
}