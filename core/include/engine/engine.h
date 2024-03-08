#pragma once

#include <SDL.h>

#include <vector>

#include "engine/system.h"
#include "proto/config.pb.h"
#include "renderer/texture.h"
#include "renderer/model.h"
#include "utils/job_system.h"
#include "engine/filesystem.h"

#include <glm/ext/vector_uint2.hpp>

namespace core
{

class OnEventInterface
{
public:
    virtual ~OnEventInterface() = default;
    virtual void OnEvent(SDL_Event& event) = 0;
};

class OnGuiInterface
{
public:
    virtual ~OnGuiInterface() = default;
    virtual void OnGui() = 0;
};

class Engine
{
public:
    virtual ~Engine() = default;
    Engine();
    void Run();
    
    void RegisterEventObserver(OnEventInterface* eventInterface);
    void RegisterOnGuiInterface(OnGuiInterface* imguiDrawInterface);
    void RegisterSystem(System* system);

    void DisableImGui();

    void SetWindowName(std::string_view windowName);
    glm::uvec2 GetWindowSize() const;
    virtual TextureManager& GetTextureManager() = 0;
    ModelManager& GetModelManager();

    enum class JobIndex
    {
        EVENT,
        PRE_UPDATE,
        UPDATE,
        PRE_IMGUI,
        IMGUI_DRAW,
        POST_IMGUI,
        SWAP_WINDOW,
        LENGTH
    };
    std::weak_ptr<Job> GetJob(JobIndex index);

protected:
    virtual void Begin();
    virtual void End();
    virtual void ResizeWindow(glm::uvec2) = 0;
    virtual void PreUpdate() = 0;
    virtual void PreImGuiDraw() = 0;
    virtual void PostImGuiDraw() = 0;
    virtual void SwapWindow() = 0;



    SDL_Window* window_ = nullptr;
    pb::Config config_;
    inline static constexpr core::Path configFilename = "config.bin";

    std::array<std::shared_ptr<Job>, (int)JobIndex::LENGTH> jobs_;
private:
    core::ModelManager modelManager_;
    core::JobSystem jobSystem_;
    std::vector<System*> systems_;
    std::vector<OnEventInterface*> onEventInterfaces;
    std::vector<OnGuiInterface*> imguiDrawInterfaces;
}; 

glm::uvec2 GetWindowSize();

TextureManager& GetTextureManager();
ModelManager& GetModelManager();

void SetWindowName(std::string_view windowName);
} // namespace core

