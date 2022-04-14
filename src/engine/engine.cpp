#include "engine/engine.h"

#include <cassert>
#include <GL/glew.h>
#include <glm/ext/vector_uint2.hpp>

#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>


namespace gpr5300
{
void Engine::Begin()
{

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
    // Set our OpenGL version.
#ifdef _MSC_VER
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    window_ = SDL_CreateWindow(
        "GPR5300",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        1280,
        720,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
    );
    glRenderContext_ = SDL_GL_CreateContext(window_);
    SDL_GL_SetSwapInterval(1);

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

    for(auto* system: systems_)
    {
        system->Begin();
    }
}

void Engine::Run()
{
    Begin();
    bool isOpen = true;
    while(isOpen)
    {
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
                switch(event.window.type)
                {
                case SDL_WINDOWEVENT_CLOSE:
                    isOpen = false;
                    break;
                case SDL_WINDOWEVENT_RESIZED:
                    glm::uvec2 newWindowSize;
                    newWindowSize.x = event.window.data1;
                    newWindowSize.y = event.window.data2;
                    glViewport(0, 0, newWindowSize.x, newWindowSize.y);
                    //TODO update current windowSize
                    break;
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
        }
        for(auto* system : systems_)
        {
            //TODO calculate delta time
            system->Update(0.0f);
        }

        //Generate new ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window_);
        ImGui::NewFrame();

        for(auto* imguiDrawInterface : imguiDrawInterfaces)
        {
            imguiDrawInterface->DrawImGui();
        }
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window_);
    }
    End();
}

void Engine::End()
{
    for (auto* system : systems_)
    {
        system->End();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(glRenderContext_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
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
}
