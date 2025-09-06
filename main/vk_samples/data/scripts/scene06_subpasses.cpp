#include "wasm/neko2.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <array>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "wasm/draw_command.h"
#include "wasm/camera.h"

#define WASM_EXPORT __attribute__((used)) __attribute__((visibility ("default")))


extern "C"
{
    void WASM_EXPORT scene06_draw(int64_t drawCommandId)
    {
        script::DrawCommand drawCommand(drawCommandId);
        const auto camera = script::GetSceneCamera();
        const auto projection = glm::perspective(glm::radians(45.0f), get_aspect(), 0.1f, 100.0f);

        if (drawCommand.EqualsName("CubeCommand"))
        {        drawCommand.Bind();
            drawCommand.SetMat4("ubo.model", glm::mat4(1.0f));
            drawCommand.SetMat4("ubo.view", camera.GetView());
            drawCommand.SetMat4("ubo.projection", projection);
            drawCommand.Draw();
        }
        else if (drawCommand.EqualsName("SphereCommand"))
        {        drawCommand.Bind();
            drawCommand.SetMat4("ubo.model", glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0,0)));
            drawCommand.SetMat4("ubo.view", camera.GetView());
            drawCommand.SetMat4("ubo.projection", projection);
            drawCommand.Draw();
        }
    }
}