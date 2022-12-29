#pragma once

#include <SDL.h>

#include <vector>

#include "engine/system.h"
#include "proto/config.pb.h"
#include "renderer/texture.h"

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
        virtual ~Engine() = default;
        Engine();
        void Run();
        
        void RegisterEventObserver(OnEventInterface* eventInterface);
        void RegisterImGuiDrawInterface(ImguiDrawInterface* imguiDrawInterface);
        void RegisterSystem(System* system);


        void SetWindowName(std::string_view windowName);
        glm::uvec2 GetWindowSize() const;
        virtual TextureManager& GetTextureManager() = 0;
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
    private:
        std::vector<System*> systems_;
        std::vector<OnEventInterface*> onEventInterfaces;
        std::vector<ImguiDrawInterface*> imguiDrawInterfaces;
        inline static constexpr std::string_view configFilename = "config.bin";
    }; 

    glm::uvec2 GetWindowSize();

    TextureManager& GetTextureManager();
} // namespace gpr5300

