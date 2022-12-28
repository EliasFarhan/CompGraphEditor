#include "gl/engine.h"
#include "triangle_program.h"

#include "engine/filesystem.h"
#include <SDL_main.h>

int main(int argc, char** argv)
{
    gpr5300::DefaultFilesystem filesystem;
    gpr5300::FilesystemLocator::provide(&filesystem);
    gpr5300::gl::Engine engine;

    gpr5300::HelloTriangleProgram triangleProgram;
    engine.RegisterSystem(&triangleProgram);
    engine.RegisterImGuiDrawInterface(&triangleProgram);

    engine.Run();

    return EXIT_SUCCESS;
}
