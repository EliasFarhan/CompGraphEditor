#include "player.h"
#include "player_filesystem.h"
#include "engine/engine.h"
#include "gl/engine.h"

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
    gpr5300::PhysFilesystem physFilesystem(argv[0]);
    physFilesystem.Begin();
    gpr5300::FilesystemLocator::provide(&physFilesystem);

    gpr5300::Player player;
    gpr5300::gl::Engine engine;
    engine.RegisterImGuiDrawInterface(&player);
    engine.RegisterSystem(&player);
    engine.RegisterEventObserver(&player);
    engine.Run();
    physFilesystem.End();
    return EXIT_SUCCESS;
}
