#include "player.h"

#include "phys_filesystem.h"
#include "engine/filesystem.h"

namespace vk
{
void Player::Begin()
{
    sceneManager_.Begin();
    sceneManager_.LoadScene(&playerScene_);
}

void Player::Update(float dt)
{
    sceneManager_.Update(dt);
}

void Player::End()
{
    sceneManager_.End();
}

void Player::OnGui()
{
}

void Player::OnEvent(SDL_Event& event)
{
    sceneManager_.OnEvent(event);
}

void Player::SetScene(std::string_view path)
{
    auto& filesystem = dynamic_cast<core::PhysFilesystem&>(core::FilesystemLocator::get());
    filesystem.AddMount(path, "", 1);
    core::pb::Scene newScene;
    const auto file = filesystem.LoadFile("root.scene");
    newScene.ParseFromArray(file.data, file.length);
    playerScene_.SetScene(newScene);
    sceneLoaded_ = true;
}
}
