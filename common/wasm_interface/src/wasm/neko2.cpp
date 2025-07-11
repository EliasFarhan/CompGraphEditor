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

void set_mat4_local(int64_t drawCommand, const void* name, const void* value)
{
    auto* mat = static_cast<const glm::mat4*>(value);
    reinterpret_cast<core::DrawCommand*>(drawCommand)->SetMat4(static_cast<const char*>(name), *mat);
}

void set_mat4_host(int64_t drawCommand, const void* name, int64_t value)
{
    auto* mat = reinterpret_cast<const glm::mat4*>(value);
    reinterpret_cast<core::DrawCommand*>(drawCommand)->SetMat4(static_cast<const char*>(name), *mat);
}

float get_aspect()
{
    const auto windowSize = core::GetWindowSize();
    return static_cast<float>(windowSize.x)/static_cast<float>(windowSize.y);
}

int64_t get_scene_camera()
{
    auto* scene = core::GetCurrentScene();
    return reinterpret_cast<int64_t>(&scene->GetCamera());
}

int64_t get_view(int64_t camera)
{
    static glm::mat4 view;
    view = reinterpret_cast<core::Camera*>(camera)->GetView();
    return reinterpret_cast<int64_t>(&view);
}

int64_t get_projection(int64_t camera)
{
    static glm::mat4 projection;
    projection = reinterpret_cast<core::Camera*>(camera)->GetProjection();
    return reinterpret_cast<int64_t>(&projection);
}
