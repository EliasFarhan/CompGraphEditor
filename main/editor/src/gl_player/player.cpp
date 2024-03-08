#include "player.h"
#include "phys_filesystem.h"

#include <filesystem>
#include <imgui.h>

namespace fs = std::filesystem;

namespace gl
{
void Player::Begin()
{
    if (!sceneLoaded_)
    {
        std::function<void(std::string_view)> recursiveIterateFile = [this, &recursiveIterateFile]
        (std::string_view directoryPath)
        {
            const fs::path dir = directoryPath;
            for (const auto& entry : fs::directory_iterator(dir))
            {
                auto path = entry.path();
                if (fs::is_directory(path.string()))
                {
                    recursiveIterateFile(path.string());
                }
                else
                {
                    if (path.extension() == ".pkg")
                    {
                        scenePaths_.push_back(path.string());
                    }
                }
            }
        };
        if (fs::exists("data/"))
        {
            recursiveIterateFile("data/");
        }
    }
    sceneManager_.Begin();
    if(sceneLoaded_)
    {

        sceneManager_.LoadScene(&playerScene_);
    }
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
    if(sceneLoaded_)
        return;
    ImGui::Begin("Load Scene");
    if(ImGui::BeginCombo("Scenes", "No Scene Selected"))
    {
        for (auto& scene : scenePaths_)
        {
            if (ImGui::Selectable(scene.c_str()))
            {
                auto& filesystem = dynamic_cast<core::PhysFilesystem&>(core::FilesystemLocator::get());
                filesystem.AddMount(scene, "", 1);
                core::pb::Scene newScene;
                const auto file = filesystem.LoadFile("root.scene");
                newScene.ParseFromArray(file.data, file.size);
                playerScene_.SetScene(newScene);
                sceneManager_.LoadScene(&playerScene_);
                sceneLoaded_ = true;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::End();
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
    newScene.ParseFromArray(file.data, file.size);
    playerScene_.SetScene(newScene);
    sceneLoaded_ = true;
}
} // namespace gpr5300
