#include "renderer/camera.h"
#include <numbers>

const float PI = std::numbers::pi_v<float>;

namespace core
{
glm::mat4 Camera::GetView() const
{
    return glm::lookAt(position, position + direction, up);
}

void CameraSystem::Begin()
{

}

void CameraSystem::Update(float dt)
{
    glm::vec3 direction{};
    if(keys_[0])
    {
        direction += glm::vec3(1,0,0);
    }
    if(keys_[1])
    {
        direction -= glm::vec3(1,0,0);
    }
    if(keys_[2])
    {
        direction += glm::vec3(0,1,0);
    }
    if(keys_[3])
    {
        direction -= glm::vec3(0,1,0);
    }
    const auto horizontal = glm::normalize(glm::cross(camera_.up, camera_.direction));
    const auto vertical = glm::cross(horizontal, camera_.direction);
    const auto windowSize = GetWindowSize();
    const auto rotateX = glm::rotate(glm::mat4(1.0), mouseMotion_.x*PI, vertical);
    const auto rotateY = glm::rotate(glm::mat4(1.0), mouseMotion_.y*PI, horizontal);
    mouseMotion_ = {};
    camera_.direction = glm::vec3(rotateX*rotateY*glm::vec4(camera_.direction, 1.0));
    camera_.position += (horizontal*direction.x+camera_.direction*direction.y)*dt;
}

void CameraSystem::End()
{

}

void CameraSystem::Draw(int subpassIndex)
{
    auto* scene = GetCurrentScene();
    auto subpass = scene->GetSubpass(subpassIndex);
    for(int i = 0; i < subpass.GetDrawCommandCount(); i++)
    {
        auto command = subpass.GetDrawCommand(i);
        auto material = command.GetMaterial();
        material.GetPipeline()->
            SetMat4("view",
                glm::lookAt(camera_.position, camera_.position+camera_.direction, camera_.up));
    }
}

void CameraSystem::OnKeyDown(SDL_Keycode keycode)
{
    switch(keycode)
    {
    case SDLK_a:
        keys_[0] = true;
        break;
    case SDLK_d:
        keys_[1] = true;
        break;
    case SDLK_w:
        keys_[2] = true;
        break;
    case SDLK_s:
        keys_[3] = true;
        break;
    default:
        break;
    }
}

void CameraSystem::OnKeyUp(SDL_Keycode keycode)
{
    switch(keycode)
    {
    case SDLK_a:
        keys_[0] = false;
        break;
    case SDLK_d:
        keys_[1] = false;
        break;
    case SDLK_w:
        keys_[2] = false;
        break;
    case SDLK_s:
        keys_[3] = false;
        break;
    default:
        break;
    }
}

void CameraSystem::OnMouseMotion(glm::vec2 mouseMotion)
{
    mouseMotion_ = mouseMotion;
}
} // namespace core
