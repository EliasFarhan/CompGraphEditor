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
    for(auto* system: systems_)
    {
        system->Begin();
    }
}

void Engine::Run()
{
    Begin();
    bool isOpen = true;

    std::chrono::time_point<std::chrono::system_clock> clock = std::chrono::system_clock::now();
    while(isOpen)
    {
#ifdef TRACY_ENABLE
        ZoneScoped;
#endif
        const auto start = std::chrono::system_clock::now();
        using seconds = std::chrono::duration<float, std::ratio<1,1>>;
        const auto dt = std::chrono::duration_cast<seconds>(start - clock);
        clock = start;

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
                    auto* windowSize = config_.mutable_windowsize();
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
            ImGui_ImplSDL2_ProcessEvent(&event);
        }

        PreUpdate();
        for(auto* system : systems_)
        {
            system->Update(dt.count());
        }

        //Generate new ImGui frame
        PreImGuiDraw();

        for(auto* imguiDrawInterface : imguiDrawInterfaces)
        {
            imguiDrawInterface->DrawImGui();
        }
        PostImGuiDraw();
        

        SwapWindow();
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


    const auto& fileSystem = FilesystemLocator::get();
    fileSystem.WriteString(configFilename, config_.SerializeAsString());

}

void Engine::RegisterEventObserver(OnEventInterface* eventInterface)
{
    onEventInterfaces.push_back(eventInterface);
}

void Engine::RegisterImGuiDrawInterface(ImguiDrawInterface* imguiDrawInterface)
{
    imguiDrawInterfaces.push_back(imguiDrawInterface);
}

void Engine::RegisterSystem(System* system)
{
    systems_.push_back(system);
}
Engine::Engine()
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    instance = this;
    const auto& fileSystem = FilesystemLocator::get();

    if(fileSystem.IsRegularFile(configFilename))
    {
        const auto file = fileSystem.LoadFile(configFilename);
        config_.ParseFromString(reinterpret_cast<const char*>(file.data));
        
    }
    else
    {
        config_.set_vertical_sync(true);
        config_.set_framerate_limit(0);
        pb::Vec2i *windowSize = config_.mutable_windowsize();
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
}
glm::uvec2 Engine::GetWindowSize() const
{
    return {config_.windowsize().x(), config_.windowsize().y()};
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
}
