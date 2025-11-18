#define MODULE_NAME scene06

#include "wasm/neko2.h"
#include "wasm/buffer.h"

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include <array>

#include "wasm/draw_command.h"

struct CameraBufferObject
{
    glm::mat4 view;
    glm::mat4 projection;
};

static CameraBufferObject ubo;
static constexpr std::array positions =
{
    glm::vec3(2.0, 5.0, -15.0),
    glm::vec3(-1.5, -2.2, -2.5),
    glm::vec3(-3.8, -2.0, -12.3),
    glm::vec3(2.4, -0.4, -3.5),
    glm::vec3(-1.7, 3.0, -7.5),
    glm::vec3(1.3, -2.0, -2.5),
    glm::vec3(1.5, 2.0, -2.5),
    glm::vec3(1.5, 0.2, -1.5),
    glm::vec3(-1.3, 1.0, -1.5)
};
static std::array<glm::mat4, positions.size()> models_{};
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
        auto cameraBuffer = script::GetBuffer("camera");
        const auto view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                            glm::vec3(0.0f, 0.0f, 0.0f),
                            glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = view;
        ubo.projection = glm::perspective(glm::radians(45.0f), get_aspect(), 0.1f, 10.0f);
        ubo.projection[1][1] *= -1.0f;
        cameraBuffer.CopyData(ubo, 0);

        auto modelBuffer = script::GetBuffer("transform");
        for (size_t i = 0; i < models_.size(); ++i)
        {
            glm::mat4 modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, positions[i]);
            modelMatrix = glm::rotate(modelMatrix, t, glm::vec3(0, 0, 1));
            modelMatrix = glm::rotate(modelMatrix, t, glm::vec3(0, 1, 0));
            models_[i] = modelMatrix;
        }
        modelBuffer.CopyData(models_);

    }

    DRAW_FUNC(drawCommandId)
    {
        script::DrawCommand drawCommand(drawCommandId);
        drawCommand.Draw(models_.size());
    }
}