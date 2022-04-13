#pragma once

#include <SDL.h>

namespace gpr5300
{

    class OnEventInterface
    {
    public:
        virtual ~OnEventInterface() = default;
        virtual OnEvent(SDL_Event& event) = 0;
    };

    class Engine
    {
    public:

    private:
        SDL_Window* window_ = nullptr;
    }; 
} // namespace gpr5300

