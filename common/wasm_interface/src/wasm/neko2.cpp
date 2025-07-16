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

void set_vec3_local(int64_t drawCommand, const void* name, const void* value)
{
    auto* v = static_cast<const glm::vec3*>(value);
    reinterpret_cast<core::DrawCommand*>(drawCommand)->SetVec3(static_cast<const char*>(name), *v);
}
void set_mat4_host(int64_t drawCommand, const void* name, int64_t value)
{
    auto* mat = reinterpret_cast<const glm::mat4*>(value);
    reinterpret_cast<core::DrawCommand*>(drawCommand)->SetMat4(static_cast<const char*>(name), *mat);
}
void set_vec3_host(int64_t drawCommand, const void* name, int64_t value)
{
    auto* v = reinterpret_cast<const glm::vec3*>(value);
    reinterpret_cast<core::DrawCommand*>(drawCommand)->SetVec3(static_cast<const char*>(name), *v);
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


void fill_camera_view(int64_t camera, void* viewMat)
{
   *static_cast<glm::mat4*>(viewMat) = reinterpret_cast<core::Camera*>(camera)->GetView();
}

void fill_camera_projection(int64_t camera, void* projMat)
{
    *static_cast<glm::mat4*>(projMat) = reinterpret_cast<core::Camera*>(camera)->GetProjection();
}


int32_t get_subpass_index(int64_t drawCommand)
{
    return reinterpret_cast<core::DrawCommand*>(drawCommand)->GetSubpassIndex();
}

int64_t get_name(int64_t drawCommand)
{
    const auto* drawCommandPtr = reinterpret_cast<core::DrawCommand*>(drawCommand);
    return reinterpret_cast<int64_t>(drawCommandPtr->GetName().data());
}

int32_t name_equals(int64_t hostString, const void* localString)
{
    const std::string_view hostName = reinterpret_cast<const char*>(hostString);
    const std::string_view localName = static_cast<const char*>(localString);
    return hostName == localName;
}

void fill_camera_position(int64_t camera, void* position)
{
    *static_cast<glm::vec3*>(position) = reinterpret_cast<core::Camera*>(camera)->position;
}
void set_camera_position(int64_t camera, const void* position)
{
    reinterpret_cast<core::Camera*>(camera)->position = *static_cast<const glm::vec3*>(position);
}

void set_camera_direction(int64_t camera, const void* direction)
{
    reinterpret_cast<core::Camera*>(camera)->direction = *static_cast<const glm::vec3*>(direction);
}

void set_camera_near(int64_t camera, float near)
{
    reinterpret_cast<core::Camera*>(camera)->near = near;
}

void set_camera_far(int64_t camera, float far)
{
    reinterpret_cast<core::Camera*>(camera)->far = far;
}

void draw_instanced(int64_t drawCommand, int64_t count)
{
    auto* scene = core::GetCurrentScene();
    reinterpret_cast<core::DrawCommand*>(drawCommand)->PreDrawBind();
    scene->Draw(*reinterpret_cast<core::DrawCommand*>(drawCommand), count);
}
