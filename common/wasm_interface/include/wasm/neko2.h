#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#include <cstdint>

extern "C"
{
    void bind_draw_command(int64_t drawCommand);
    void set_float(int64_t drawCommand, const void* name, float value);
    void draw(int64_t drawCommand);
}
