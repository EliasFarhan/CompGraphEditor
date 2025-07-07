#include "wasm/neko2.h"
#include <emscripten.h>

#define GLM_ENABLE_EXPERIMENTAL
#define WASM_EXPORT __attribute__((used)) __attribute__((visibility("default")))

extern "C"
{

static float t = 0.0f;

EMSCRIPTEN_KEEPALIVE void begin()
{
    t = 0.0f;
}

EMSCRIPTEN_KEEPALIVE void update(float dt)
{
    t += dt;
}

EMSCRIPTEN_KEEPALIVE void draw(neko2::draw::DrawCommand* draw_command)
{
    neko2::draw::bind(draw_command);
    neko2::draw::set_float(draw_command, "value", (std::sin(t)+1.0f)/2.0f);
    neko2::draw::draw(draw_command);
}

EMSCRIPTEN_KEEPALIVE void end()
{

}
}