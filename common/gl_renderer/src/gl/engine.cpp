#include "gl/engine.h"

#include <GL/glew.h>
#include <glm/ext/vector_uint2.hpp>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <glm/vec2.hpp>

#include "gl/debug.h"

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

namespace gl
{
gl::TextureManager& Engine::GetTextureManager()
{
    return textureManager_;
}

void Engine::Begin()
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
    // Set our OpenGL version.
#if true
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    const auto windowSize = glm::ivec2(config_.windowsize().x(), config_.windowsize().y());
    window_ = SDL_CreateWindow(
        config_.window_name().c_str(),
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        windowSize.x,
        windowSize.y,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
    );
    glRenderContext_ = SDL_GL_CreateContext(window_);
    SDL_GL_SetSwapInterval(config_.vertical_sync());

    if (GLEW_OK != glewInit())
    {
        assert(false && "Failed to initialize OpenGL context");
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Keyboard Gamepad
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsClassic();
    ImGui_ImplSDL2_InitForOpenGL(window_, glRenderContext_);
    ImGui_ImplOpenGL3_Init("#version 300 es");

    core::Engine::Begin();
}

void Engine::End()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    core::Engine::End();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(glRenderContext_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

void Engine::ResizeWindow(glm::uvec2 windowSize)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    glViewport(0, 0, windowSize.x, windowSize.y);
}

void Engine::PreUpdate()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Engine::PreImGuiDraw()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    ImGui_ImplOpenGL3_NewFrame();
    glCheckError();
    ImGui_ImplSDL2_NewFrame(window_);
    ImGui::NewFrame();
}

void Engine::PostImGuiDraw()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glCheckError();
}

void Engine::SwapWindow()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    SDL_GL_SwapWindow(window_);
    glCheckError();
}
} // namespace gl
