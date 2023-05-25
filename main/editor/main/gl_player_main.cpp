#include "player.h"
#include "phys_filesystem.h"
#include "py_interface.h"
#include "gl/engine.h"


#include <argh.h>

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
    core::PhysFilesystem physFilesystem(argv[0]);
    physFilesystem.Begin();
    core::FilesystemLocator::provide(&physFilesystem);

    argh::parser cmdl(argv);

    core::PyManager pyManager;
    core::ImportNativeScript();
    gl::Player player;
    if (cmdl.size() == 2)
    {
        player.SetScene(cmdl[1]);
    }
    gl::Engine engine;
    int major = 0, minor = 0;
    if (cmdl({ "-M", "--major" }) >> major && cmdl({ "-m", "--minor" }) >> minor)
    {
        engine.SetVersion(major, minor, cmdl[{ "-es", "--es" }]);
    }
    engine.SetWindowName("OpenGL Scene Player");
    engine.RegisterOnGuiInterface(&player);
    engine.RegisterSystem(&player);
    engine.RegisterEventObserver(&player);
    engine.Run();
    physFilesystem.End();
    return EXIT_SUCCESS;
}
