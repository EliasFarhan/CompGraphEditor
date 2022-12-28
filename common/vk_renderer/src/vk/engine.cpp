#include "vk/engine.h"

namespace gpr5300::vk
{
void Engine::Begin()
{
    window_.Begin();

    gpr5300::Engine::Begin();
}

void Engine::End()
{
    gpr5300::Engine::End();

    window_.End();
}

void Engine::ResizeWindow(glm::uvec2 uvec2)
{

}

void Engine::PreUpdate()
{

}

void Engine::PreImGuiDraw()
{

}

void Engine::PostImGuiDraw()
{

}

void Engine::SwapWindow()
{

}

Engine::Engine() : window_(config_)
{
}
}