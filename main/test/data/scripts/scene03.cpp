#include "wasm/neko2.h"
#define GLM_ENABLE_EXPERIMENTAL
#define WASM_EXPORT __attribute__((used)) __attribute__((visibility("default")))

static float t = 0.0f;

void WASM_EXPORT begin()
{

}

void WASM_EXPORT update(float dt)
{
    t += dt;
}

void WASM_EXPORT draw(neko2::draw::DrawCommand* draw_command)
{
    neko2::draw::bind(draw_command);
    neko2::draw::set_float(draw_command, "value", (std::sin(t)+1.0f)/2.0f);
    neko2::draw::draw(draw_command);
}

void WASM_EXPORT end()
{

}