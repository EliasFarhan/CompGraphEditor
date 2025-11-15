#define MODULE_NAME scene04

#include "wasm/neko2.h"
#include "wasm/buffer.h"

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include <array>

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

static UniformBufferObject ubo;

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
        ubo.model = glm::rotate(glm::mat4(1.0f),
            t * glm::radians(90.0f),
            glm::vec3(0.0f, 0.0f, 1.0f));
        auto buffer = script::GetBuffer("ubo");
        const auto view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                            glm::vec3(0.0f, 0.0f, 0.0f),
                            glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = view;
        ubo.projection = glm::perspective(glm::radians(45.0f), get_aspect(), 0.1f, 10.0f);
        ubo.projection[1][1] *= -1.0f;
        buffer.CopyData(ubo, 0);

    }
}