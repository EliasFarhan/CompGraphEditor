#include "renderer/command.h"

namespace core
{
glm::mat4 ModelTransformMatrix::GetModelTransformMatrix() const
{
    auto transform = glm::mat4_cast(glm::quat(glm::radians(rotation_)));
    transform = glm::scale(glm::mat4(1.0f), scale_) * transform;
    transform = glm::translate(glm::mat4(1.0f), translate_) * transform;
    return transform;
}
}