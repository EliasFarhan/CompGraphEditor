#include "gl/engine.h"
#include "sample_program.h"

#include "engine/filesystem.h"
#include <SDL_main.h>

#include "py_interface.h"

int main([[maybe_unused]]int argc, [[maybe_unused]] char** argv)
{
    core::DefaultFilesystem filesystem;
    core::FilesystemLocator::provide(&filesystem);
    gl::Engine engine;
    core::PyManager pyManager;

    gpr5300::SampleBrowserProgram triangleProgram;
    engine.RegisterSystem(&triangleProgram);
    engine.RegisterOnGuiInterface(&triangleProgram);
    engine.RegisterEventObserver(&triangleProgram);

    engine.Run();

    return EXIT_SUCCESS;
}
