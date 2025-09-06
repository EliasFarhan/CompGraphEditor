
#include <argh.h>

#include "engine/filesystem.h"
#include "vk/engine.h"

#include <SDL3/SDL_main.h>

#include "wasm_interface.h"
#include "py_interface.h"
#include "sample_program.h"

int main([[maybe_unused]] int argc, char** argv)
{
    argh::parser cmdl(argv);
    vk::Engine engine;
    int major = 0, minor = 0;
    if (cmdl({ "-M", "--major" }) >> major && cmdl({ "-m", "--minor" }) >> minor)
    {
        engine.SetVersion(major, minor);
    }
    engine.DisableImGui();
    engine.SetWindowName("Vulkan Samples");
    core::WasmManager wasmManager;
    core::ImportNativeScript();

    gpr5300::HelloVulkanProgram vulkanProgram;
    engine.RegisterSystem(&vulkanProgram);
    engine.RegisterEventObserver(&vulkanProgram);

    engine.Run();

    return EXIT_SUCCESS;
}
