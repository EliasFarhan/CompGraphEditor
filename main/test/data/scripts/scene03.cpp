#include <emscripten.h>
#include "wasm/neko2.h"

#include <cmath>
#include <cstdio>
#include <string_view>

#define WASM_EXPORT __attribute__((used)) __attribute__((visibility ("default")))
#define GLM_ENABLE_EXPERIMENTAL

extern "C"
{

static float t = 0.0f;

EMSCRIPTEN_KEEPALIVE void WASM_EXPORT scene03_begin()
{
    t = 0.0f;
}

EMSCRIPTEN_KEEPALIVE void WASM_EXPORT scene03_update(float dt)
{
    t += dt;
}

EMSCRIPTEN_KEEPALIVE void WASM_EXPORT scene03_draw(int64_t drawCommand)
{
    bind_draw_command(drawCommand);
    float value = (std::sin(t)+1.0f)/2.0f;
    static constexpr std::string_view uniform_name = "value";
    set_float(drawCommand, uniform_name.data(), value);
    draw(drawCommand);
}

EMSCRIPTEN_KEEPALIVE void WASM_EXPORT scene03_end()
{

}
}