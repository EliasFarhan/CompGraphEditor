#include <SDL3/SDL_main.h>
#include <argh.h>

#include "novus/engine.h"
#include "phys_filesystem.h"
#include "gpu_player/player.h"
#include "wasm_interface.h"
//
// Created by unite on 06.11.2025.
//
int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
    core::PhysFilesystem physFilesystem(argv[0]);
    physFilesystem.Begin();
    core::SetFileSystem(&physFilesystem);

    core::WasmManager wasmManager;
    core::ImportNativeScript();
    argh::parser cmdl(argv);
    novus::Player player;
    if (cmdl.size() == 2)
    {
        player.SetScene(cmdl[1]);
    }
    novus::Engine engine;
    engine.SetWindowName("Novus Scene Player");
    engine.RegisterOnGuiInterface(&player);
    engine.RegisterSystem(&player);
    engine.RegisterEventObserver(&player);
    engine.Run();
    physFilesystem.End();
    return EXIT_SUCCESS;
    return 0;
}