
#include <argh.h>

#include "phys_filesystem.h"
#include "player.h"
#include "py_interface.h"
#include "engine/script.h"
#include "vk/engine.h"

int main([[maybe_unused]]int argc, char** argv)
{
    core::PhysFilesystem physFilesystem(argv[0]);
    physFilesystem.Begin();
    core::FilesystemLocator::provide(&physFilesystem);

    argh::parser cmdl(argv);
    vk::Engine engine;
    int major = 0, minor = 0;
    if (cmdl({ "-M", "--major" }) >> major && cmdl({ "-m", "--minor" }) >> minor)
    {
        engine.SetVersion(major, minor);
    }

    engine.DisableImGui();
    engine.SetWindowName("Vulkan Scene Player");

    core::PyManager pyManager;
    core::ImportNativeScript();
    vk::Player player;
    if (cmdl.size() >= 2)
    {
        player.SetScene(cmdl[1]);
    }
    else
    {
        LogError("First argument is the scene pkg file!");
        return EXIT_FAILURE;
    }
    engine.RegisterSystem(&player);
    engine.RegisterEventObserver(&player);

    engine.Run();
    physFilesystem.End();

    return EXIT_SUCCESS;
}
