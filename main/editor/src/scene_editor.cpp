#include "scene_editor.h"
#include <imgui.h>
#include <SDL.h>
#include <pybind11/embed.h>
#include <fmt/format.h>
#include <engine/filesystem.h>

namespace gpr5300
{

void Editor::DrawImGui()
{
    auto* engine = Engine::GetInstance();
    const auto windowSize = engine->GetWindowSize();


    ImGui::SetNextWindowPos(ImVec2(0,0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(windowSize.x*0.2f, windowSize.y), ImGuiCond_FirstUseEver);
    DrawSceneContent();

    ImGui::SetNextWindowPos(ImVec2(windowSize.x*0.2f,0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(windowSize.x*0.6f, windowSize.y), ImGuiCond_FirstUseEver);
    ImGui::Begin("Center View", nullptr, ImGuiWindowFlags_NoTitleBar);
    DrawMenuBar();
    DrawCenterView();
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(windowSize.x*0.8f,0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(windowSize.x*0.2f, windowSize.y), ImGuiCond_FirstUseEver);
    DrawInspector();

    ImGui::SetNextWindowPos(ImVec2(0,windowSize.y*0.6f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(windowSize.x, windowSize.y*0.4f), ImGuiCond_FirstUseEver);
    DrawLogWindow();

    UpdateFileDialog();
}
void Editor::Begin()
{
    py::initialize_interpreter();
    auto& filesystem = FilesystemLocator::get();
    if(!filesystem.IsDirectory(ResourceManager::dataFolder))
    {
        auto os = py::module_::import("os");
        os.attr("mkdir")(ResourceManager::dataFolder);
    }
    else
    {
        //TODO Recursively add the file to the editor
        resourceManager_.CheckDataFolder();
    }
}
void Editor::Update(float dt)
{

}
void Editor::End()
{
    py::finalize_interpreter();
}
void Editor::DrawMenuBar()
{
    if(ImGui::BeginMainMenuBar())
    {
        if(ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("Open"))
            {
                fileDialog_.Open();
            }
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Window"))
        {
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}
void Editor::DrawSceneContent()
{
    ImGui::Begin("Scene Content");

    if(ImGui::TreeNode("Shaders"))
    {
        ImGui::TreePop();
    }

    if(ImGui::TreeNode("Pipelines"))
    {
        ImGui::TreePop();
    }

    if(ImGui::TreeNode("Materials"))
    {
        ImGui::TreePop();
    }

    if(ImGui::TreeNode("Textures"))
    {
        ImGui::TreePop();
    }
    ImGui::End();
}
void Editor::DrawCenterView()
{
    if(ImGui::BeginTabBar("Center View"))
    {
        if(ImGui::BeginTabItem("Shader"))
        {
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Pipeline"))
        {
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Material"))
        {
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Texture"))
        {
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}
void Editor::DrawInspector()
{
    ImGui::Begin("Inspector");
    ImGui::End();
}
void Editor::UpdateFileDialog()
{
    fileDialog_.Display();

    if(fileDialog_.HasSelected())
    {
        const auto path = fileDialog_.GetSelected().string();
        LogDebug(fmt::format("Selected filename: {}", path));
        LoadFileIntoEditor(path);
        fileDialog_.ClearSelected();
    }
}
void Editor::DrawLogWindow()
{
    const auto& logs = GetLogs();
    ImGui::Begin("Log");
    for(const auto& log : logs)
    {
        switch(log.type)
        {
        case Log::Type::Error:
            ImGui::TextColored(ImColor(255,0,0,255), "%s", log.msg.c_str());
            break;
        case Log::Type::Warning:
            ImGui::TextColored(ImColor(255,255,0,255), "%s", log.msg.c_str());
            break;
        case Log::Type::Debug:
            ImGui::TextColored(ImColor(150,150,150,255), "%s", log.msg.c_str());
            break;
        default:
            break;
        }
    }
    ImGui::End();

}
void Editor::OnEvent(SDL_Event &event)
{
    switch(event.type)
    {
    case SDL_WINDOWEVENT:
    {
        switch(event.window.event)
        {
        case SDL_WINDOWEVENT_FOCUS_GAINED:
        {
            //TODO check if data content changed
            break;
        }
        }
        break;
    }
    }

}
void Editor::LoadFileIntoEditor(std::string_view path)
{
    //TODO Copy the file in path into the data folder depending on the category

    /*
    const auto extension = GetFileExtension(path);
    static constexpr std::array<std::string_view, 3> shaderExtensions =
        {
        ".vert",
        ".frag",
        ".comp"
        };
    if(std::ranges::any_of(shaderExtensions, [&extension](auto& shaderExtension){
        return extension == shaderExtension;
    }))
    {
        py::function analyzeShaderFunc = py::module_::import("scripts.shader_parser").attr("analyze_shader");
        try
        {
            std::string result = (py::str) analyzeShaderFunc(path);
            LogDebug(fmt::format("Loading shader: {} with content:\n{}", path, result));
        }
        catch (py::error_already_set& e)
        {
            LogError(fmt::format("Analyze shader failed\n{}", e.what()));
        }
    }
     */
}
void Editor::DrawEditorContent()
{
    ImGui::Begin("Editor Content");

    if(ImGui::TreeNode("Shaders"))
    {
        ImGui::TreePop();
    }

    if(ImGui::TreeNode("Pipelines"))
    {
        ImGui::TreePop();
    }

    if(ImGui::TreeNode("Materials"))
    {
        ImGui::TreePop();
    }

    if(ImGui::TreeNode("Textures"))
    {
        ImGui::TreePop();
    }
    ImGui::End();
}
}