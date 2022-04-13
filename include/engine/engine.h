#pragma once

#include <SDL.h>

#include <vector>

namespace gpr5300
{

    class OnEventInterface
    {
    public:
        virtual ~OnEventInterface() = default;
        virtual OnEvent(SDL_Event& event) = 0;
    };

    class ImguiDrawInterface
    {
    public:
        virtual ~ImguiDrawInterface() = default;
        virtual DrawImGui() = 0;
    }

    class Engine
    {
    public:

        void Begin();
        void Run();
        void End();
        
        void RegisterEventObserver(OnEventInterface* eventInterface);
        void RegisterImGuiDrawInterface(ImguiDrawInterface* imguiDrawInterface);
    private:
        SDL_Window* window_ = nullptr;
        std::vector<OnEventInterface*> onEventInterfaces;
        std::vector<ImguiDrawInterface*> imguiDrawInterfaces;
    }; 
} // namespace gpr5300

