//
// Created by unite on 06.11.2025.
//

#ifndef NEKO2_PLAYER_H
#define NEKO2_PLAYER_H

#include "novus/scene.h"

namespace novus
{
class Player : public core::OnGuiInterface, public core::System, public core::OnEventInterface
{
public:
    void Begin() override;
    void Update(float dt) override;
    void End() override;
    void OnGui() override;
    void OnEvent(SDL_Event& event) override;
    void SetScene(std::string_view path);

private:
    core::SceneManager sceneManager_;
    std::vector<std::string> scenePaths_;
    bool sceneLoaded_ = false;
    novus::Scene playerScene_;
};
}
#endif // NEKO2_PLAYER_H
