#include <emscripten.h>
#include "wasm/neko2.h"

#include <cmath>
#include <string_view>

#include "wasm/draw_command.h"

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

void WASM_EXPORT scene03_draw(int64_t drawCommandId)
{
    script::DrawCommand drawCommand(drawCommandId);
    drawCommand.Bind();
    drawCommand.SetFloat("value", (std::sin(t)+1.0f)/2.0f);
    drawCommand.Draw();
}
}