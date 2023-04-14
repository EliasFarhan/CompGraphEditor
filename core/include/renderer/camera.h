#pragma once

#include "engine/system.h"

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>


namespace core
{
    
struct Camera
{
    enum class ProjectionType
    {
        ORTHOGRAPHIC,
        PERSPECTIVE,
        NONE
    };
    glm::vec3 position = {0,0,-10};
    glm::vec3 direction = {0,0,1};
    glm::vec3 up = {0,1,0};
    float near = 0.1f;
    float far = 100.0f;

    ProjectionType projectionType = ProjectionType::PERSPECTIVE;
    float fovY = 45.0f;
    float aspect = 1.0f; // x / y
    float orthographicHalfHeight = 1.0f;

    [[nodiscard]] glm::mat4 GetView() const;
    [[nodiscard]] glm::mat4 GetProjection() const;
};

class CameraSystem final : public Script
{
public:
    CameraSystem();
    void Begin() override;
    void Draw(DrawCommand* drawCommand) override;
    void OnKeyDown(SDL_Keycode keycode) override;
    void OnKeyUp(SDL_Keycode keycode) override;
    void OnMouseMotion(glm::vec2 mouseMotion) override;

    void Update(float dt) override;

    void End() override;
    Camera& GetCamera() {return camera_;}
    [[nodiscard]] const Camera& GetCamera() const {return camera_;}
private:
    Camera camera_{};
    glm::vec2 mouseMotion_{};
    std::array<bool, 4> keys_{};

};

} // namespace core
