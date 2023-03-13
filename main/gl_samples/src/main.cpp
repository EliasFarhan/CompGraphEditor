#include "gl/engine.h"
#include "sample_program.h"

#include "engine/filesystem.h"
#include <SDL_main.h>

int main([[maybe_unused]]int argc, [[maybe_unused]] char** argv)
{
    core::DefaultFilesystem filesystem;
    core::FilesystemLocator::provide(&filesystem);
    gl::Engine engine;

    gpr5300::SampleBrowserProgram triangleProgram;
    engine.RegisterSystem(&triangleProgram);
    engine.RegisterImGuiDrawInterface(&triangleProgram);
    engine.RegisterEventObserver(&triangleProgram);

    engine.Run();

    return EXIT_SUCCESS;
}
