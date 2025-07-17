#pragma once


#include "wasm/neko2.h"

namespace script
{

class Camera
{
    public:
    explicit Camera(int64_t cameraId): cameraId_(cameraId)
    {

    }
    void SetPosition(const glm::vec3& position) const
    {
        set_camera_position(cameraId_, &position.x);
    }
    void SetDirection(const glm::vec3& direction) const
    {
        set_camera_direction(cameraId_, &direction.x);
    }
    void SetFar(float far) const
    {
        set_camera_far(cameraId_, far);
    }
    [[nodiscard]] glm::vec3 GetPosition() const
    {
        glm::vec3 cameraPos;
        fill_camera_position(cameraId_, &cameraPos);
        return cameraPos;
    }

    [[nodiscard]] glm::mat4 GetView() const
    {
        glm::mat4 cameraView;
        fill_camera_view(cameraId_, &cameraView);
        return cameraView;
    }

    [[nodiscard]] glm::mat4 GetProjection() const
    {
        glm::mat4 cameraProj;
        fill_camera_projection(cameraId_, &cameraProj);
        return cameraProj;
    }

private:
    int64_t cameraId_;
};

inline Camera GetSceneCamera()
{
    return Camera{get_scene_camera()};
}

inline Camera GetSystemCamera()
{
    return Camera{get_system_camera()};
}

}
