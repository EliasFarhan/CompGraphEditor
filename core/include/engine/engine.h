#pragma once

#include <SDL3/SDL.h>

#include <vector>

#include "engine/system.h"
#include "renderer/texture.h"
#include "renderer/model.h"
#include "thread/job_system.h"
#include "generated/engine_generated.h"

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
    novus::engine::ShaderFormat GetShaderFormat();
    /*
     * \note This cannot be called after RunEngine()
     */
    void SetShaderFormat(novus::engine::ShaderFormat shaderFormat);

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
    neko::Job* GetJob(JobIndex index);

protected:
    virtual void Begin();
    virtual void End();
    virtual void ResizeWindow(glm::uvec2) = 0;
    virtual void PreUpdate() = 0;
    virtual void PreImGuiDraw() = 0;
    virtual void PostImGuiDraw() = 0;
    virtual void SwapWindow() = 0;



    SDL_Window* window_ = nullptr;
    novus::engine::ConfigT config_;
    static constexpr std::string_view configFilename = "config.bin";

    std::array<std::unique_ptr<neko::Job>, (int)JobIndex::LENGTH> jobs_;
private:
    class EventJob final : public neko::Job
    {
    public:
        explicit EventJob(Engine* engine, bool& isOpen);
        void ExecuteImpl() override;
    private:
        Engine* engine_;
        bool& isOpen_;
    };
    friend class EventJob;
    class PreUpdateJob final : public neko::DependentJob
    {
    public:
        explicit PreUpdateJob(Job* parentJob, Engine* engine);
        void ExecuteImpl() override;
    private:
        Engine* engine_;
    };
    friend class PreUpdateJob;
    class UpdateJob final : public neko::DependentJob
    {
    public:
        using seconds = std::chrono::duration<float, std::ratio<1,1>>;
        explicit UpdateJob(Job* parentJob, Engine* engine, const seconds& dt);
        void ExecuteImpl() override;
    private:
        Engine* engine_;
        const seconds& dt_;
    };
    friend class UpdateJob;
    class PreImGuiJob final : public neko::DependentJob
    {
    public:
        explicit PreImGuiJob(Job* parentJob, Engine* engine);
        void ExecuteImpl() override;
    private:
        Engine* engine_;
    };
    class ImGuiDrawJob final : public neko::DependentJob
    {
    public:
        explicit ImGuiDrawJob(Job* parentJob, Engine* engine);
        void ExecuteImpl() override;
    private:
        Engine* engine_;
    };
    friend class ImGuiDrawJob;
    class PostImGuiJob final : public neko::DependentJob
    {
    public:
        explicit PostImGuiJob(Job* parentJob, Engine* engine);
        void ExecuteImpl() override;
    private:
        Engine* engine_;
    };
    friend class PostImGuiJob;
    class SwapWindowJob : public neko::DependentJob
    {
    public:
        explicit SwapWindowJob(Job* parentJob, Engine* engine);
        void ExecuteImpl() override;
    private:
        Engine* engine_;
    };
    friend class SwapWindowJob;
    core::ModelManager modelManager_;
    std::vector<System*> systems_;
    std::vector<OnEventInterface*> onEventInterfaces;
    std::vector<OnGuiInterface*> imguiDrawInterfaces_;
}; 


constexpr SDL_GPUShaderFormat ConvertShaderFormat(novus::engine::ShaderFormat shaderFormat)
{
    switch(shaderFormat)
    {
    case novus::engine::ShaderFormat_SPIRV:
        return SDL_GPU_SHADERFORMAT_SPIRV;
        break;
    case novus::engine::ShaderFormat_DXIL:
        return SDL_GPU_SHADERFORMAT_DXIL;
        break;
    case novus::engine::ShaderFormat_MSL:
        return SDL_GPU_SHADERFORMAT_MSL;
        break;
    case novus::engine::ShaderFormat_METALLIB:
        return SDL_GPU_SHADERFORMAT_METALLIB;
        break;
    case novus::engine::ShaderFormat_DXBC:
        return SDL_GPU_SHADERFORMAT_DXBC;
        break;
    }
    return SDL_GPU_SHADERFORMAT_INVALID;
}

glm::uvec2 GetWindowSize();
novus::engine::ShaderFormat GetShaderFormat();
TextureManager& GetTextureManager();
ModelManager& GetModelManager();

void SetWindowName(std::string_view windowName);
} // namespace core

