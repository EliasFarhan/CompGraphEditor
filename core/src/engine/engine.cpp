#include "engine/engine.h"
//#include "renderer/debug.h"

#include "engine/filesystem.h"


#include <chrono>
#include <cassert>
#include <imgui_impl_sdl3.h>
#include <glm/vec2.hpp>

#include "utils/log.h"

#include <format>


#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

namespace core
{

static Engine* instance = nullptr;

ModelManager& Engine::GetModelManager()
{
    return modelManager_;
}
novus::engine::ShaderFormat Engine::GetShaderFormat()
{
    return config_.shader_format;
}

void Engine::Begin()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    neko::JobSystem::Begin();
    for(auto* system: systems_)
    {
        system->Begin();
    }
}

void Engine::Run()
{
    Begin();
    bool isOpen = true;

    jobs_[(int)JobIndex::EVENT] = std::make_unique<EventJob>(this, isOpen);

    jobs_[(int)JobIndex::PRE_UPDATE] = std::make_unique<PreUpdateJob>(jobs_[(int)JobIndex::EVENT].get(), this);
    using seconds = std::chrono::duration<float, std::ratio<1,1>>;
    seconds dt;
    jobs_[(int)JobIndex::UPDATE]  = std::make_unique<UpdateJob>(jobs_[(int)JobIndex::PRE_UPDATE].get(), this, dt);

    jobs_[(int)JobIndex::PRE_IMGUI] = std::make_unique<PreImGuiJob>(jobs_[(int)JobIndex::UPDATE].get(), this);

    jobs_[(int)JobIndex::IMGUI_DRAW]  = std::make_unique<ImGuiDrawJob>(jobs_[(int)JobIndex::PRE_IMGUI].get(), this);

    jobs_[(int)JobIndex::POST_IMGUI]  = std::make_unique<PostImGuiJob>(jobs_[(int)JobIndex::IMGUI_DRAW].get(), this);

    jobs_[(int)JobIndex::SWAP_WINDOW] = std::make_unique<SwapWindowJob>(jobs_[(int)JobIndex::POST_IMGUI].get(), this);

    std::chrono::time_point<std::chrono::system_clock> clock = std::chrono::system_clock::now();
    while(isOpen)
    {
#ifdef TRACY_ENABLE
        ZoneScoped;
#endif
        const auto start = std::chrono::system_clock::now();

        dt = std::chrono::duration_cast<seconds>(start - clock);
        clock = start;

        for(auto& job: jobs_)
        {
            neko::JobSystem::AddJob(job.get());
        }
        neko::JobSystem::ExecuteMainThread();
#ifdef TRACY_ENABLE
        FrameMark;
#endif
    }
    End();
}

void Engine::End()
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    for (auto* system : systems_)
    {
        system->End();
    }

    neko::JobSystem::End();
    flatbuffers::FlatBufferBuilder fbb;
    fbb.Finish(novus::engine::Config::Pack(fbb, &config_));
    std::string configData{reinterpret_cast<const char*>(fbb.GetBufferPointer()), fbb.GetSize()};
    WriteString(configFilename, configData);

}
Engine::EventJob::EventJob(Engine* engine, bool& isOpen):
    engine_(engine), isOpen_(isOpen) {}
void Engine::EventJob::ExecuteImpl()
{
    //Manage SDL event
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
        case SDL_EVENT_QUIT:
            isOpen_ = false;
            break;

        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            isOpen_ = false;
            break;
        case SDL_EVENT_WINDOW_RESIZED: {
            glm::uvec2 newWindowSize;
            newWindowSize.x = event.window.data1;
            newWindowSize.y = event.window.data2;
            engine_->ResizeWindow(newWindowSize);
            engine_->config_.window_size = glm::ivec2(newWindowSize.x, newWindowSize.y);
            break;
        }
        default:
            break;
        }
        for(auto* eventInterface: engine_->onEventInterfaces)
        {
            eventInterface->OnEvent(event);
        }
        if (!engine_->config_.no_imgui)
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
        }
    }
}
Engine::PreUpdateJob::PreUpdateJob(Job* parentJob, Engine* engine): engine_(engine), DependentJob(parentJob) {}
void Engine::PreUpdateJob::ExecuteImpl()
{
    engine_->PreUpdate();
}
Engine::UpdateJob::UpdateJob(Job* parentJob, Engine* engine, const seconds& dt): engine_(engine), dt_(dt), DependentJob(parentJob) {}
void Engine::UpdateJob::ExecuteImpl()
{
    for(auto* system : engine_->systems_)
    {
        system->Update(dt_.count());
    }
}
Engine::PreImGuiJob::PreImGuiJob(Job* parentJob, Engine* engine):engine_(engine), DependentJob(parentJob) {}
void Engine::PreImGuiJob::ExecuteImpl()
{
    engine_->PreImGuiDraw();
}
Engine::ImGuiDrawJob::ImGuiDrawJob(Job* parentJob, Engine* engine):engine_(engine), DependentJob(parentJob) {}
void Engine::ImGuiDrawJob::ExecuteImpl()
{
    for(auto* imguiDrawInterface : engine_->imguiDrawInterfaces_)
    {
        imguiDrawInterface->OnGui();
    }
}
Engine::PostImGuiJob::PostImGuiJob(Job* parentJob, Engine* engine):engine_(engine), DependentJob(parentJob) {}
void Engine::PostImGuiJob::ExecuteImpl()
{
    engine_->PostImGuiDraw();
}
Engine::SwapWindowJob::SwapWindowJob(Job* parentJob, Engine* engine): engine_(engine), DependentJob(parentJob) {}
void Engine::SwapWindowJob::ExecuteImpl()
{
    engine_->SwapWindow();
}

void Engine::RegisterEventObserver(OnEventInterface* eventInterface)
{
    onEventInterfaces.push_back(eventInterface);
}

void Engine::RegisterOnGuiInterface(OnGuiInterface* imguiDrawInterface)
{
    imguiDrawInterfaces_.push_back(imguiDrawInterface);
}

void Engine::RegisterSystem(System* system)
{
    systems_.push_back(system);
}

void Engine::DisableImGui()
{
    config_.no_imgui = true;
}

Engine::Engine()
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    instance = this;
    SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG);
    if(IsRegularFile(configFilename))
    {
        const auto file = LoadFile(configFilename);
        auto config = flatbuffers::GetRoot<novus::engine::Config>(file.data);
        config->UnPackTo(&config_);
    }
    else
    {
        config_.vertical_sync = (true);
        config_.framerate_limit = (0);
        config_.window_size = glm::ivec2(1280, 720);
        config_.window_name = "CompGraphEditor";
        config_.fullscreen = (false);
        config_.enable_debug = (true);
    }
}
void Engine::SetWindowName(std::string_view windowName)
{
    config_.window_name = (windowName.data());
    if(window_ != nullptr)
    {
        SDL_SetWindowTitle(window_, windowName.data());
    }
}
glm::uvec2 Engine::GetWindowSize() const
{
    return {config_.window_size.x, config_.window_size.y};
}

neko::Job* Engine::GetJob(Engine::JobIndex index)
{
    return jobs_[(int)index].get();
}

glm::uvec2 GetWindowSize()
{
    return instance->GetWindowSize();
}
novus::engine::ShaderFormat GetShaderFormat()
{
    return instance->GetShaderFormat();
}
TextureManager& GetTextureManager()
{
    return instance->GetTextureManager();
}

ModelManager& GetModelManager()
{
    return instance->GetModelManager();
}

void SetWindowName(std::string_view windowName)
{
    instance->SetWindowName(windowName);
}
}
