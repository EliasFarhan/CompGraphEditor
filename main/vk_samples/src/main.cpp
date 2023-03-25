//
// Created by efarhan on 12/28/22.
//
#include "engine/filesystem.h"
#include "vk/engine.h"

#include <SDL_main.h>

#include "sample_program.h"

int main(int argc, char** argv)
{
    core::DefaultFilesystem filesystem;
    core::FilesystemLocator::provide(&filesystem);
    vk::Engine engine;

    gpr5300::HelloVulkanProgram triangleProgram;
    engine.RegisterSystem(&triangleProgram);
    engine.RegisterOnGuiInterface(&triangleProgram);

    engine.Run();

    return EXIT_SUCCESS;
}
