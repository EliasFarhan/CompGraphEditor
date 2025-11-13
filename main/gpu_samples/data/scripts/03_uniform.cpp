#define MODULE_NAME scene03


#include "wasm/neko2.h"
#include "wasm/draw_command.h"

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include <array>

extern "C"
{
    static float t = 0.0f;

    BEGIN_FUNC()
    {
        t = 0.0f;
    }

    UPDATE_FUNC(dt)
    {
        t += dt;
    }

    DRAW_FUNC(drawCommandId)
    {
        script::DrawCommand drawCommand(drawCommandId);
        drawCommand.SetFloat("ratio", (std::sin(t)+1.0f)/2.0f);
        drawCommand.Draw();
    }
}