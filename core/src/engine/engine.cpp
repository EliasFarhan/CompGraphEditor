#include "engine/engine.h"
//#include "renderer/debug.h"

#include "engine/filesystem.h"


#include <chrono>
#include <cassert>
#include <imgui_impl_sdl2.h>
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
    jobSystem_.Begin();
    for(auto* system: systems_)
    {
        system->Begin();
    }
}

void Engine::Run()
{
    Begin();
    bool isOpen = true;

    jobs_[(int)JobIndex::EVENT] = std::make_shared<FuncJob>([this, &isOpen](){
        //Manage SDL event
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    isOpen = false;
                    break;
                case SDL_WINDOWEVENT:
                {
                    switch(event.window.event)
                    {
                        case SDL_WINDOWEVENT_CLOSE:
                            isOpen = false;
                            break;
                        case SDL_WINDOWEVENT_RESIZED:
                        {
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
                ImGui_ImplSDL2_ProcessEvent(&event);
            }
        }
    });

    jobs_[(int)JobIndex::PRE_UPDATE] = std::make_shared<FuncDependentJob>(jobs_[(int)JobIndex::EVENT], [this](){
        PreUpdate();
    });
    using seconds = std::chrono::duration<float, std::ratio<1,1>>;
    seconds dt;
    jobs_[(int)JobIndex::UPDATE]  = std::make_shared<FuncDependentJob>(jobs_[(int)JobIndex::PRE_UPDATE], [this, &dt](){
        for(auto* system : systems_)
        {
            system->Update(dt.count());
        }
    });

    jobs_[(int)JobIndex::PRE_IMGUI] = std::make_shared<FuncDependentJob>(jobs_[(int)JobIndex::UPDATE], [this](){
        //Generate new ImGui frame
        PreImGuiDraw();
    });

    jobs_[(int)JobIndex::IMGUI_DRAW]  = std::make_shared<FuncDependentJob>(jobs_[(int)JobIndex::PRE_IMGUI], [this](){
        for(auto* imguiDrawInterface : imguiDrawInterfaces)
        {
            imguiDrawInterface->OnGui();
        }
    });

    jobs_[(int)JobIndex::POST_IMGUI]  = std::make_shared<FuncDependentJob>(jobs_[(int)JobIndex::IMGUI_DRAW] , [this](){
        PostImGuiDraw();
    });

    jobs_[(int)JobIndex::SWAP_WINDOW] = std::make_shared<FuncDependentJob>(jobs_[(int)JobIndex::POST_IMGUI], [this](){
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
            jobSystem_.AddJob(job);
        }
        jobSystem_.ExecuteMainThread();
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

    jobSystem_.End();
    const auto& fileSystem = FilesystemLocator::get();
    fileSystem.WriteString(Path(configFilename), config_.SerializeAsString());

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
    const auto& fileSystem = FilesystemLocator::get();

    if(fileSystem.IsRegularFile(Path(configFilename)))
    {
        const auto file = fileSystem.LoadFile(Path(configFilename));
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

std::weak_ptr<Job> Engine::GetJob(Engine::JobIndex index)
{
    return jobs_[(int)index];
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
