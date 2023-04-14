//
// Created by efarhan on 12/28/22.
//
#include "engine/filesystem.h"
#include "vk/engine.h"

#include <SDL_main.h>

#include "py_interface.h"
#include "sample_program.h"

int main(int argc, char** argv)
{
    core::DefaultFilesystem filesystem;
    core::FilesystemLocator::provide(&filesystem);
    vk::Engine engine;
    core::PyManager pyManager;

    gpr5300::HelloVulkanProgram vulkanProgram;
    engine.RegisterSystem(&vulkanProgram);
    engine.RegisterOnGuiInterface(&vulkanProgram);
    engine.RegisterEventObserver(&vulkanProgram);

    engine.Run();

    return EXIT_SUCCESS;
}
