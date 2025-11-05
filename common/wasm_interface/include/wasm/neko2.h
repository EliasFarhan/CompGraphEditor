#pragma once

#include <glm/mat4x4.hpp>

#include "renderer/camera.h"

extern "C"
{
    void bind_draw_command(int64_t drawCommand);
    void set_float(int64_t drawCommand, const void* name, float value);
    void set_mat4_local(int64_t drawCommand, const void* name, const void* value);
    void set_vec3_local(int64_t drawCommand, const void* name, const void* value);
    void set_mat4_host(int64_t drawCommand, const void* name, int64_t value);
    void set_vec3_host(int64_t drawCommand, const void* name, int64_t value);
    void draw(int64_t drawCommand);
    void draw_instanced(int64_t drawCommand, int64_t count);
    float get_aspect();
    int64_t get_scene_camera();
    int64_t get_system_camera();
    void fill_camera_view(int64_t camera, void* viewMat);
    void fill_camera_projection(int64_t camera, void* projMat);
    void fill_camera_position(int64_t camera, void* position);
    int32_t get_subpass_index(int64_t drawCommand);

    int64_t get_name(int64_t drawCommand);
    int32_t name_equals(int64_t hostString, const void* localString);
    void set_camera_position(int64_t camera, const void* position);
    void set_camera_direction(int64_t camera, const void* direction);
    void set_camera_near(int64_t camera, float near);
    void set_camera_far(int64_t camera, float far);

    //int64_t get_buffer(const void* bufferName);
    //void buffer_copy_data(int64_t bufferName, const void* data, int64_t size);
}

inline void set_vec3(int64_t drawCommand, const void* name, const glm::vec3& value)
{
	set_vec3_local(drawCommand, name, &value);
}

template<typename T>
void set_vec3(int64_t drawCommand, const void* name, T value)
{
    if constexpr  (std::is_pointer_v<T>)
    {
        set_vec3_local(drawCommand, name, value);
    }
    else if constexpr (std::is_convertible_v<std::remove_cvref_t<T>, glm::vec3>)
    {
        set_vec3_local(drawCommand, name, &value);
    }
    else if constexpr (std::is_same_v<T, int64_t>)
    {
        set_vec3_host(drawCommand, name, value);
    }
}

inline void set_mat4(int64_t drawCommand, const void* name, const glm::mat4& value)
{
	set_mat4_local(drawCommand, name, &value);
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
    else if constexpr (std::is_convertible_v<std::remove_cvref_t<T>, glm::mat4>)
    {
        set_mat4_local(drawCommand, name, &value);
    }
}

