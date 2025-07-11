#pragma once

#include <glm/mat4x4.hpp>
#include <cstdint>

#include "renderer/camera.h"

extern "C"
{
    void bind_draw_command(int64_t drawCommand);
    void set_float(int64_t drawCommand, const void* name, float value);
    void set_mat4_local(int64_t drawCommand, const void* name, const void* value);
    void set_mat4_host(int64_t drawCommand, const void* name, int64_t value);
    void draw(int64_t drawCommand);
    float get_aspect();
    int64_t get_scene_camera();
    int64_t get_view(int64_t camera);
    int64_t get_projection(int64_t camera);
}

template<typename T>
void set_mat4(int64_t drawCommand, const void* name, T value)
{
    if constexpr  (std::is_pointer_v<T>)
    {
        set_mat4_local(drawCommand, name, value);
    }
    else if constexpr (std::is_same_v<T, int64_t>)
    {
        set_mat4_host(drawCommand, name, value);
    }

}


