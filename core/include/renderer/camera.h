#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace core
{
    
struct Camera
{
    glm::vec3 position = {0,0,-10};
    glm::vec3 direction = {0,0,1};
    glm::vec3 up = {0,1,0};

    [[nodiscard]] glm::mat4 GetView() const;
};

} // namespace core
