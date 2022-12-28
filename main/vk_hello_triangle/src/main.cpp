//
// Created by efarhan on 12/28/22.
//
#include "engine/filesystem.h"
#include "vk/engine.h"

#include <SDL_main.h>

int main(int argc, char** argv)
{
    gpr5300::DefaultFilesystem filesystem;
    gpr5300::FilesystemLocator::provide(&filesystem);
    gpr5300::vk::Engine engine;

    //gpr5300::HelloTriangleProgram triangleProgram;
    //engine.RegisterSystem(&triangleProgram);
    //engine.RegisterImGuiDrawInterface(&triangleProgram);

    engine.Run();

    return EXIT_SUCCESS;
}
