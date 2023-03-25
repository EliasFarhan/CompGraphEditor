#pragma once

#include "engine/py_interface.h"

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

class CameraSystem final : public Script
{
public:
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
    std::array<bool, 4> keys_;

};

} // namespace core
