//
// Created by efarhan on 12/28/22.
//
#include <argh.h>

#include "engine/filesystem.h"
#include "vk/engine.h"

#include <SDL_main.h>

#include "py_interface.h"
#include "sample_program.h"

int main([[maybe_unused]] int argc, char** argv)
{
    argh::parser cmdl(argv);
    core::DefaultFilesystem filesystem;
    core::FilesystemLocator::provide(&filesystem);
    vk::Engine engine;
    int major = 0, minor = 0;
    if (cmdl({ "-M", "--major" }) >> major && cmdl({ "-m", "--minor" }) >> minor)
    {
        engine.SetVersion(major, minor);
    }
    core::PyManager pyManager;
    core::ImportNativeScript();

    gpr5300::HelloVulkanProgram vulkanProgram;
    engine.RegisterSystem(&vulkanProgram);
    engine.RegisterOnGuiInterface(&vulkanProgram);
    engine.RegisterEventObserver(&vulkanProgram);

    engine.Run();

    return EXIT_SUCCESS;
}
