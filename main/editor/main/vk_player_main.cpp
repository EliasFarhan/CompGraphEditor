
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

    core::PyManager pyManager;
    core::ImportNativeScript();
    vk::Player player;
    if (cmdl.size() == 2)
    {
        player.SetScene(cmdl[1]);
    }
    else
    {
        return EXIT_FAILURE;
    }
    vk::Engine engine;
    int major = 0, minor = 0;
    if (cmdl({ "-M", "--major" }) >> major && cmdl({ "-m", "--minor" }) >> minor)
    {
        engine.SetVersion(major, minor);
    }
    engine.RegisterOnGuiInterface(&player);
    engine.RegisterSystem(&player);
    engine.RegisterEventObserver(&player);
    engine.Run();
    physFilesystem.End();
    return EXIT_SUCCESS;
}
