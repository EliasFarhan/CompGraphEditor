#include <emscripten.h>
#include "wasm/neko2.h"

#include <cmath>
#include <cstdio>

#define GLM_ENABLE_EXPERIMENTAL

extern "C"
{

static float t = 0.0f;

EMSCRIPTEN_KEEPALIVE void scene03_begin()
{
    t = 0.0f;
}

EMSCRIPTEN_KEEPALIVE void scene03_update(float dt)
{
    t += dt;
}

EMSCRIPTEN_KEEPALIVE void scene03_draw(int64_t draw_command)
{
    bind_draw_command(draw_command);
    set_float(draw_command, "value", (std::sin(t)+1.0f)/2.0f);
    draw(draw_command);
}

EMSCRIPTEN_KEEPALIVE void scene03_end()
{

}
}