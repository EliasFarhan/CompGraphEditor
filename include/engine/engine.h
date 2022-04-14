#pragma once

#include <SDL.h>

#include <vector>

#include "system.h"


namespace gpr5300
{

    class OnEventInterface
    {
    public:
        virtual ~OnEventInterface() = default;
        virtual void OnEvent(SDL_Event& event) = 0;
    };

    class ImguiDrawInterface
    {
    public:
        virtual ~ImguiDrawInterface() = default;
        virtual void DrawImGui() = 0;
    };

    class Engine
    {
    public:

        void Run();
        
        void RegisterEventObserver(OnEventInterface* eventInterface);
        void RegisterImGuiDrawInterface(ImguiDrawInterface* imguiDrawInterface);
        void RegisterSystem(System* system);
    private:
        void Begin();
        void End();

        SDL_Window* window_ = nullptr;

        std::vector<System*> systems_;
        std::vector<OnEventInterface*> onEventInterfaces;
        std::vector<ImguiDrawInterface*> imguiDrawInterfaces;
        SDL_GLContext glRenderContext_{};
    }; 
} // namespace gpr5300

