#define MODULE_NAME scene04

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
        /*
        auto viewBuffer = GetBuffer("view");
        auto projectionBuffer = GetBuffer("projection");
        auto view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(0,0,-5));
        viewBuffer.SetMat4("view", view);
        auto projection = glm::perspective(glm::radians(45.0f), get_aspect(), 0.1f, 100.0f);
        drawCommand.SetMat4("projection", projection);
        */
    }
}