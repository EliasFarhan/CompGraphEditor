#include "editor.h"
#include "shader_editor.h"
#include "utils/log.h"
#include <imgui.h>
#include <SDL.h>
#include <pybind11/embed.h>
#include <fmt/format.h>
#include "engine/filesystem.h"
#include "editor_filesystem.h"

namespace gpr5300
{

void Editor::Begin()
{
    editorSystems_.resize(static_cast<std::size_t>(EditorType::LENGTH));
    editorSystems_[static_cast<std::size_t>(EditorType::SHADER)] = std::make_unique<ShaderEditor>();


    resourceManager_.RegisterResourceChange(this);
    py::initialize_interpreter();
    auto& filesystem = FilesystemLocator::get();
    if(!filesystem.IsDirectory(ResourceManager::dataFolder))
    {
        CreateDirectory(ResourceManager::dataFolder);

    }
    else
    {
        resourceManager_.CheckDataFolder();
    }
    for(const auto& editorSystem: editorSystems_)
    {
        if(!editorSystem)
            continue;
        const auto subFolder = fmt::format("{}{}", ResourceManager::dataFolder, editorSystem->GetSubFolder());
        if(!filesystem.IsDirectory(subFolder))
            CreateDirectory(subFolder);
    }
}

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

    EditorSystem* editorSystem = FindEditorSystem(path);
    if(editorSystem == nullptr)
    {
        LogError(fmt::format("Could not find appropriated editor system for file: {}", path));
        return;
    }
    auto dstPath = fmt::format("{}{}{}",
                               ResourceManager::dataFolder,
                               editorSystem->GetSubFolder(),
                               GetFilename(path));
    if(CopyFile(path, dstPath))
    {
        resourceManager_.AddResource(dstPath);
    }
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

void Editor::AddResource(const Resource &resource)
{
    EditorSystem* editorSystem = FindEditorSystem(resource.path);
    if(editorSystem == nullptr)
        return;
    editorSystem->AddResource(resource);
}

void Editor::RemoveResource(const Resource &resource)
{
    EditorSystem* editorSystem = FindEditorSystem(resource.path);
    if(editorSystem == nullptr)
        return;
    editorSystem->RemoveResource(resource);
}

void Editor::UpdateResource(const Resource &resource)
{
    EditorSystem* editorSystem = FindEditorSystem(resource.path);
    if(editorSystem == nullptr)
        return;
    editorSystem->UpdateResource(resource);
}

EditorSystem *Editor::FindEditorSystem(std::string_view path)
{
    EditorSystem* editorSystem = nullptr;
    const auto extension = GetFileExtension(path);
    for(auto& editorSystemTmp : editorSystems_)
    {
        if(!editorSystemTmp)
            continue;
        if(!editorSystemTmp->CheckExtensions(extension))
            continue;
        editorSystem = editorSystemTmp.get();
        break;
    }
    return editorSystem;
}
}