#pragma once

#include <cstdint>

extern "C"
{
    void bind_draw_command(int64_t drawCommand);
    void set_float(int64_t drawCommand, const void* name, float value);
    void set_mat4(int64_t drawCommand, const void* name, const void* value);
    void draw(int64_t drawCommand);
    float get_aspect();
}
