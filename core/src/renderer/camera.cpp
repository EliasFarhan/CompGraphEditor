#include "renderer/camera.h"

namespace core
{
glm::mat4 Camera::GetView() const
{
    return glm::lookAt(position, position + direction, up);
}
} // namespace core
