#include "wasm/neko2.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <array>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "wasm/draw_command.h"

#define MODULE_NAME scene03
#define WASM_EXPORT __attribute__((used)) __attribute__((visibility ("default")))

#define BEGIN_FUNC() void WASM_EXPORT MODULE_NAME ## begin()
#define DRAW_FUNC(drawCommand) void WASM_EXPORT MODULE_NAME ## draw(int64_t drawCommandId)
#define UPDATE_FUNC(dt) void WASM_EXPORT MODULE_NAME ## update(float dt)

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
        drawCommand.Bind();
        auto view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(0,0,-5));
        drawCommand.SetMat4("view", view);
        auto projection = glm::perspective(glm::radians(45.0f), get_aspect(), 0.1f, 100.0f);
        drawCommand.SetMat4("projection", projection);
        //add model matrix
    }
}