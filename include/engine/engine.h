#pragma once

#include <SDL.h>

#include <vector>

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
    private:
        void Begin();
        void End();

        SDL_Window* window_ = nullptr;
        std::vector<OnEventInterface*> onEventInterfaces;
        std::vector<ImguiDrawInterface*> imguiDrawInterfaces;
        SDL_GLContext glRenderContext_{};
    }; 
} // namespace gpr5300

