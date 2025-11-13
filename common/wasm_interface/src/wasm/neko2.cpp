#include "wasm/neko2.h"
#include "renderer/command.h"
#include "engine/scene.h"
#include "renderer/buffer.h"

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif


void set_float(int64_t drawCommand, const void* name, float value)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    reinterpret_cast<core::DrawCommand*>(drawCommand)->SetFloat(static_cast<const char*>(name), value);
}

void draw(int64_t drawCommand)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    auto* scene = core::GetCurrentScene();
    reinterpret_cast<core::DrawCommand*>(drawCommand)->PreDrawBind();
    scene->Draw(*reinterpret_cast<core::DrawCommand*>(drawCommand));
}

void set_mat4_local(int64_t drawCommand, const void* name, const void* value)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    auto* mat = static_cast<const glm::mat4*>(value);
    reinterpret_cast<core::DrawCommand*>(drawCommand)->SetMat4(static_cast<const char*>(name), *mat);
}

void set_vec3_local(int64_t drawCommand, const void* name, const void* value)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    auto* v = static_cast<const glm::vec3*>(value);
    reinterpret_cast<core::DrawCommand*>(drawCommand)->SetVec3(static_cast<const char*>(name), *v);
}
void set_mat4_host(int64_t drawCommand, const void* name, int64_t value)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    auto* mat = reinterpret_cast<const glm::mat4*>(value);
    reinterpret_cast<core::DrawCommand*>(drawCommand)->SetMat4(static_cast<const char*>(name), *mat);
}
void set_vec3_host(int64_t drawCommand, const void* name, int64_t value)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    auto* v = reinterpret_cast<const glm::vec3*>(value);
    reinterpret_cast<core::DrawCommand*>(drawCommand)->SetVec3(static_cast<const char*>(name), *v);
}

float get_aspect()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    const auto windowSize = core::GetWindowSize();
    return static_cast<float>(windowSize.x)/static_cast<float>(windowSize.y);
}

int64_t get_scene_camera()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    auto* scene = core::GetCurrentScene();
    return reinterpret_cast<int64_t>(&scene->GetCamera());
}


void fill_camera_view(int64_t camera, void* viewMat)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
   *static_cast<glm::mat4*>(viewMat) = reinterpret_cast<core::Camera*>(camera)->GetView();
}

void fill_camera_projection(int64_t camera, void* projMat)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    *static_cast<glm::mat4*>(projMat) = reinterpret_cast<core::Camera*>(camera)->GetProjection();
}


int32_t get_subpass_index(int64_t drawCommand)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    return reinterpret_cast<core::DrawCommand*>(drawCommand)->GetSubpassIndex();
}

int64_t get_name(int64_t drawCommand)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    const auto* drawCommandPtr = reinterpret_cast<core::DrawCommand*>(drawCommand);
    return reinterpret_cast<int64_t>(drawCommandPtr->GetName().data());
}

int32_t name_equals(int64_t hostString, const void* localString)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    const std::string_view hostName = reinterpret_cast<const char*>(hostString);
    const std::string_view localName = static_cast<const char*>(localString);
    return hostName == localName;
}

void fill_camera_position(int64_t camera, void* position)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    *static_cast<glm::vec3*>(position) = reinterpret_cast<core::Camera*>(camera)->position;
}
void set_camera_position(int64_t camera, const void* position)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    reinterpret_cast<core::Camera*>(camera)->position = *static_cast<const glm::vec3*>(position);
}

void set_camera_direction(int64_t camera, const void* direction)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    reinterpret_cast<core::Camera*>(camera)->direction = *static_cast<const glm::vec3*>(direction);
}

void set_camera_near(int64_t camera, float near)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    reinterpret_cast<core::Camera*>(camera)->near = near;
}

void set_camera_far(int64_t camera, float far)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    reinterpret_cast<core::Camera*>(camera)->far = far;
}

void draw_instanced(int64_t drawCommand, int64_t count)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    auto* scene = core::GetCurrentScene();
    reinterpret_cast<core::DrawCommand*>(drawCommand)->PreDrawBind();
    scene->Draw(*reinterpret_cast<core::DrawCommand*>(drawCommand), count);
}
int64_t get_system_camera()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    auto* cameraSystem = core::GetCameraSystem();
    if (cameraSystem)
    {
        return reinterpret_cast<int64_t>(&cameraSystem->camera);
    }
    return 0;
}
/*
int64_t get_buffer(const void* bufferName)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    auto& bufferManager = core::GetCurrentScene()->GetBufferManager();
    return bufferManager.GetBuffer(static_cast<const char*>(bufferName)).bufferId;
}
void buffer_copy_data(int64_t buffer, const void* data, int64_t size)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    auto& bufferManager = core::GetCurrentScene()->GetBufferManager();
    auto arrayBuffer = bufferManager.GetArrayBuffer(static_cast<core::BufferId>(buffer));
    if (size > arrayBuffer.count*arrayBuffer.typeSize)
    {
        LogError("Trying to copy buffer out of range");
        std::terminate();
    }
    std::memcpy(arrayBuffer.data, data, size);
}

*/