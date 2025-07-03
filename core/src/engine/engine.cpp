#include "engine/engine.h"
//#include "renderer/debug.h"

#include "engine/filesystem.h"


#include <chrono>
#include <cassert>
#include <imgui_impl_sdl3.h>
#include <glm/vec2.hpp>

#include "utils/log.h"

#include <fmt/format.h>

#include "proto/vector.pb.h"

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

    jobs_[(int)JobIndex::EVENT] = std::make_unique<neko::FuncJob>([this, &isOpen](){
        //Manage SDL event
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_EVENT_QUIT:
                    isOpen = false;
                    break;

                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                    isOpen = false;
                    break;
                case SDL_EVENT_WINDOW_RESIZED: {
                    glm::uvec2 newWindowSize;
                    newWindowSize.x = event.window.data1;
                    newWindowSize.y = event.window.data2;
                    ResizeWindow(newWindowSize);
                    auto* windowSize = config_.mutable_window_size();
                    windowSize->set_x(newWindowSize.x);
                    windowSize->set_y(newWindowSize.y);
                    break;
                }
                default:
                    break;
            }
            for(auto* eventInterface: onEventInterfaces)
            {
                eventInterface->OnEvent(event);
            }
            if (!config_.no_imgui())
            {
                ImGui_ImplSDL3_ProcessEvent(&event);
            }
        }
    });

    jobs_[(int)JobIndex::PRE_UPDATE] = std::make_unique<neko::FuncDependentJob>(jobs_[(int)JobIndex::EVENT].get(), [this](){
        PreUpdate();
    });
    using seconds = std::chrono::duration<float, std::ratio<1,1>>;
    seconds dt;
    jobs_[(int)JobIndex::UPDATE]  = std::make_unique<neko::FuncDependentJob>(jobs_[(int)JobIndex::PRE_UPDATE].get(), [this, &dt](){
        for(auto* system : systems_)
        {
            system->Update(dt.count());
        }
    });

    jobs_[(int)JobIndex::PRE_IMGUI] = std::make_unique<neko::FuncDependentJob>(jobs_[(int)JobIndex::UPDATE].get(), [this](){
        //Generate new ImGui frame
        PreImGuiDraw();
    });

    jobs_[(int)JobIndex::IMGUI_DRAW]  = std::make_unique<neko::FuncDependentJob>(jobs_[(int)JobIndex::PRE_IMGUI].get(), [this](){
        for(auto* imguiDrawInterface : imguiDrawInterfaces)
        {
            imguiDrawInterface->OnGui();
        }
    });

    jobs_[(int)JobIndex::POST_IMGUI]  = std::make_unique<neko::FuncDependentJob>(jobs_[(int)JobIndex::IMGUI_DRAW].get() , [this](){
        PostImGuiDraw();
    });

    jobs_[(int)JobIndex::SWAP_WINDOW] = std::make_unique<neko::FuncDependentJob>(jobs_[(int)JobIndex::POST_IMGUI].get(), [this](){
       SwapWindow();
    });

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
    WriteString(configFilename, config_.SerializeAsString());

}

void Engine::RegisterEventObserver(OnEventInterface* eventInterface)
{
    onEventInterfaces.push_back(eventInterface);
}

void Engine::RegisterOnGuiInterface(OnGuiInterface* imguiDrawInterface)
{
    imguiDrawInterfaces.push_back(imguiDrawInterface);
}

void Engine::RegisterSystem(System* system)
{
    systems_.push_back(system);
}

void Engine::DisableImGui()
{
    config_.set_no_imgui(true);
}

Engine::Engine()
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    instance = this;

    if(IsRegularFile(configFilename))
    {
        const auto file = LoadFile(configFilename);
        config_.ParseFromString(reinterpret_cast<const char*>(file.data));
        
    }
    else
    {
        config_.set_vertical_sync(true);
        config_.set_framerate_limit(0);
        pb::Vec2i *windowSize = config_.mutable_window_size();
        windowSize->set_x(1280);
        windowSize->set_y(720);
        config_.set_window_name("CompGraphEditor");
        config_.set_fullscreen(false);
        config_.set_enable_debug(true);
    }
}
void Engine::SetWindowName(std::string_view windowName)
{
    config_.set_window_name(windowName.data());
    if(window_ != nullptr)
    {
        SDL_SetWindowTitle(window_, windowName.data());
    }
}
glm::uvec2 Engine::GetWindowSize() const
{
    return {config_.window_size().x(), config_.window_size().y()};
}

neko::Job* Engine::GetJob(Engine::JobIndex index)
{
    return jobs_[(int)index].get();
}

glm::uvec2 GetWindowSize()
{
    return instance->GetWindowSize();
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
