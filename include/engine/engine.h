#pragma once

#include <SDL.h>

#include <vector>

#include "engine/system.h"
#include "proto/config.pb.h"

#include <glm/ext/vector_uint2.hpp>

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
        Engine();
        void Run();
        
        void RegisterEventObserver(OnEventInterface* eventInterface);
        void RegisterImGuiDrawInterface(ImguiDrawInterface* imguiDrawInterface);
        void RegisterSystem(System* system);

        void SetWindowName(std::string_view windowName);
        glm::uvec2 GetWindowSize() const;
        static Engine* GetInstance() { return engine_; }
    private:
        void Begin();
        void End();

        inline static Engine* engine_ = nullptr;

        pb::Config config_;
        SDL_Window* window_ = nullptr;

        std::vector<System*> systems_;
        std::vector<OnEventInterface*> onEventInterfaces;
        std::vector<ImguiDrawInterface*> imguiDrawInterfaces;
        SDL_GLContext glRenderContext_{};
        inline static constexpr std::string_view configFilename = "config.bin";
    }; 
} // namespace gpr5300

