#include "player.h"
#include "player_filesystem.h"
#include "engine/engine.h"

int main(int argc, char** argv)
{
    gpr5300::PhysFilesystem physFilesystem(argv[0]);
    physFilesystem.Begin();
    gpr5300::FilesystemLocator::provide(&physFilesystem);

    gpr5300::Player player;
    gpr5300::Engine engine;
    engine.RegisterImGuiDrawInterface(&player);
    engine.RegisterSystem(&player);
    engine.Run();
    physFilesystem.End();
    return EXIT_SUCCESS;
}