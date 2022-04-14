#include "engine/engine.h"
#include "triangle_program.h"
#include <SDL_main.h>

int main(int argc, char** argv)
{
    gpr5300::Engine engine;
    gpr5300::HelloTriangleProgram triangleProgram;
    engine.RegisterSystem(&triangleProgram);

    engine.Run();

    return EXIT_SUCCESS;
}