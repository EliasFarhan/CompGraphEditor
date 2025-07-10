#include "wasm/neko2.h"
#include "renderer/command.h"
#include "engine/scene.h"


void bind_draw_command(int64_t drawCommand)
{
    reinterpret_cast<core::DrawCommand*>(drawCommand)->Bind();
}

void set_float(int64_t drawCommand, const void* name, float value)
{
    reinterpret_cast<core::DrawCommand*>(drawCommand)->SetFloat(static_cast<const char*>(name), value);
}

void draw(int64_t drawCommand)
{
    auto* scene = core::GetCurrentScene();
    reinterpret_cast<core::DrawCommand*>(drawCommand)->PreDrawBind();
    scene->Draw(*reinterpret_cast<core::DrawCommand*>(drawCommand));
}
