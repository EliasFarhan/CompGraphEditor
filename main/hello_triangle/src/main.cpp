#include "engine/engine.h"
#include <SDL_main.h>

int main(int argc, char** argv)
{
    gpr5300::Engine engine;

    engine.Run();

    return EXIT_SUCCESS;
}