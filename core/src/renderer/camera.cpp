#include "renderer/camera.h"
#include "renderer/draw_command.h"
#include <numbers>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

const float PI = std::numbers::pi_v<float>;

namespace core
{
glm::mat4 Camera::GetView() const
{
    return glm::lookAt(position, position + direction, up);
}

glm::mat4 Camera::GetProjection() const
{
    switch(projectionType)
    {
    case ProjectionType::ORTHOGRAPHIC: return glm::ortho(-aspect*orthographicHalfHeight, aspect*orthographicHalfHeight, -orthographicHalfHeight, orthographicHalfHeight, near, far);
    case ProjectionType::PERSPECTIVE: return glm::perspective(fovY, aspect, near, far);
    default: break;
    }
    return glm::mat4(1.0f);
}


static CameraSystem* cameraSystem = nullptr;
CameraSystem* GetCameraSystem()
{
    return cameraSystem;
}

void CameraSystem::Begin()
{
    cameraSystem = this;
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
    const auto horizontal = glm::normalize(glm::cross(camera.up, camera.direction));
    const auto vertical = glm::cross(horizontal, camera.direction);
    const auto windowSize = GetWindowSize();
    const auto rotateX = glm::rotate(glm::mat4(1.0), mouseMotion_.x*PI/static_cast<float>(windowSize.x), vertical);
    const auto rotateY = glm::rotate(glm::mat4(1.0), mouseMotion_.y*PI/static_cast<float>(windowSize.y), horizontal);
    mouseMotion_ = {};
    camera.direction = glm::vec3(rotateX*rotateY*glm::vec4(camera.direction, 1.0));
    camera.position += (horizontal*direction.x+camera.direction*direction.y)*dt;

    //Updating the scene camera
    auto* scene = GetCurrentScene();
    scene->GetCamera() = camera;
}

void CameraSystem::End()
{
    camera = {};
}


void CameraSystem::Draw(DrawCommand* drawCommand)
{
    
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
