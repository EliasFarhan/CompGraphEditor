#include "player.h"
#include "player_filesystem.h"
#include "py_interface.h"
#include "engine/engine.h"
#include "gl/engine.h"

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
    gpr5300::PhysFilesystem physFilesystem(argv[0]);
    physFilesystem.Begin();
    core::FilesystemLocator::provide(&physFilesystem);

    core::PyManager pyManager;
    core::ImportNativeScript();
    gpr5300::Player player;
    gl::Engine engine;
    engine.RegisterOnGuiInterface(&player);
    engine.RegisterSystem(&player);
    engine.RegisterEventObserver(&player);
    engine.Run();
    physFilesystem.End();
    return EXIT_SUCCESS;
}
