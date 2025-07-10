#include <emscripten.h>
#include "wasm/neko2.h"

#include <cmath>
#include <string_view>

#define WASM_EXPORT __attribute__((used)) __attribute__((visibility ("default")))
#define GLM_ENABLE_EXPERIMENTAL

extern "C"
{

static float t = 0.0f;

void WASM_EXPORT scene03_begin()
{
    t = 0.0f;
}

void WASM_EXPORT scene03_update(float dt)
{
    t += dt;
}

void WASM_EXPORT scene03_draw(int64_t drawCommand)
{
    bind_draw_command(drawCommand);
    set_float(drawCommand, "value", (std::sin(t)+1.0f)/2.0f);
    draw(drawCommand);
}

void WASM_EXPORT scene03_end()
{

}
}