#include "editor.h"
#include "shader_editor.h"
#include "utils/log.h"
#include <imgui.h>
#include <SDL.h>
#include <pybind11/embed.h>
#include <fmt/format.h>
#include "engine/filesystem.h"
#include "editor_filesystem.h"

#include "imnodes.h"
#include "pipeline_editor.h"

namespace gpr5300
{

void Editor::Begin()
{
    ImNodes::CreateContext();

    editorSystems_.resize(static_cast<std::size_t>(EditorType::LENGTH));
    editorSystems_[static_cast<std::size_t>(EditorType::SHADER)] = std::make_unique<ShaderEditor>();
    editorSystems_[static_cast<std::size_t>(EditorType::PIPELINE)] = std::make_unique<PipelineEditor>();

    resourceManager_.RegisterResourceChange(this);
    py::initialize_interpreter();
    const auto& filesystem = FilesystemLocator::get();
    if (!filesystem.IsDirectory(ResourceManager::dataFolder))
    {
        CreateDirectory(ResourceManager::dataFolder);
    }
    else
    {
        resourceManager_.CheckDataFolder();
    }
    for (const auto& editorSystem : editorSystems_)
    {
        if (!editorSystem)
            continue;
        const auto subFolder = fmt::format("{}{}", ResourceManager::dataFolder, editorSystem->GetSubFolder());
        if (!filesystem.IsDirectory(subFolder))
            CreateDirectory(subFolder);
    }
}

Editor::Editor()
{
    instance_ = this;
}

void Editor::DrawImGui()
{
    auto* engine = Engine::GetInstance();
    const auto windowSize = engine->GetWindowSize();

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(windowSize.x * 0.2f, windowSize.y), ImGuiCond_FirstUseEver);
    DrawEditorContent();

    ImGui::SetNextWindowPos(ImVec2(windowSize.x * 0.2f, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(windowSize.x * 0.6f, windowSize.y), ImGuiCond_FirstUseEver);
    ImGui::Begin("Center View", nullptr, ImGuiWindowFlags_NoTitleBar);
    DrawMenuBar();
    DrawCenterView();
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(windowSize.x * 0.8f, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(windowSize.x * 0.2f, windowSize.y), ImGuiCond_FirstUseEver);
    DrawInspector();

    ImGui::SetNextWindowPos(ImVec2(0, windowSize.y * 0.6f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(windowSize.x, windowSize.y * 0.4f), ImGuiCond_FirstUseEver);
    DrawLogWindow();

    UpdateFileDialog();
}
void Editor::Update(float dt)
{

}
void Editor::End()
{
    py::finalize_interpreter();

    ImNodes::DestroyContext();
}

void Editor::SaveProject()
{
    for(auto& editor : editorSystems_)
    {
        if(editor)
        {
            editor->Save();
        }
    }
}

void Editor::DrawMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open"))
            {
                OpenFileBrowserDialog(FileBrowserMode::OPEN_FILE);
            }
            if(ImGui::MenuItem("Save"))
            {
                SaveProject();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Window"))
        {
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void Editor::DrawSceneContent()
{
    ImGui::Begin("Scene Content");

    if (ImGui::TreeNode("Shaders"))
    {
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Pipelines"))
    {
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Materials"))
    {
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Textures"))
    {
        ImGui::TreePop();
    }
    ImGui::End();
}
void Editor::DrawCenterView()
{
    if (ImGui::BeginTabBar("Center View"))
    {
        ImGuiTabItemFlags_ flag = ImGuiTabItemFlags_None;
        flag = currentFocusedSystem_ == EditorType::PIPELINE ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;

        if (ImGui::BeginTabItem("Pipeline", nullptr, flag))
        {
            editorSystems_[static_cast<int>(EditorType::PIPELINE)]->DrawMainView();
            ImGui::EndTabItem();
        }
        flag = currentFocusedSystem_ == EditorType::SHADER ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
        if (ImGui::BeginTabItem("Shader", nullptr, flag))
        {
            editorSystems_[static_cast<int>(EditorType::SHADER)]->DrawMainView();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Material"))
        {
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Texture"))
        {
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}
void Editor::DrawInspector()
{
    ImGui::Begin("Inspector");
    if (currentFocusedSystem_ != EditorType::LENGTH)
    {
        auto* editorSystem = editorSystems_[static_cast<int>(currentFocusedSystem_)].get();
        if (editorSystem)
        {
            editorSystem->DrawInspector();
        }
    }
    ImGui::End();
}
void Editor::UpdateFileDialog()
{
    fileDialog_.Display();

    if (fileDialog_.HasSelected())
    {
        const auto& filesystem = FilesystemLocator::get();
        const auto path = fileDialog_.GetSelected().string();
        switch(fileBrowserMode_)
        {
        case FileBrowserMode::OPEN_FILE:
        {
            LogDebug(fmt::format("Selected filename: {}", path));
            LoadFileIntoEditor(path);
            break;
        }
        case FileBrowserMode::CREATE_NEW_SHADER:
        {
            const auto extension = GetFileExtension(path);
            if (!editorSystems_[(int) EditorType::SHADER]->CheckExtensions(extension))
            {
                LogWarning(fmt::format("Invalid extension name for newly created shader: {} path: {}",
                                       extension,
                                       path));
                break;
            }

            filesystem.WriteString(path, "#version 310 es\nprecision highp float;\nvoid main() {}");
            resourceManager_.AddResource(path);
            break;
        }
        case FileBrowserMode::CREATE_NEW_PIPELINE:
        {
            const auto extension = GetFileExtension(path);
            if (!editorSystems_[(int) EditorType::PIPELINE]->CheckExtensions(extension))
            {
                LogWarning(fmt::format("Invalid extension name for newly created pipeline: {} path: {}",
                                       extension,
                                       path));
                break;
            }
            pb::Pipeline emptyPipeline;
            filesystem.WriteString(path, emptyPipeline.SerializeAsString());
            resourceManager_.AddResource(path);
            break;
        }
        default:
            break;
        }
        fileDialog_.ClearSelected();
    }
}
void Editor::DrawLogWindow()
{
    const auto& logs = GetLogs();
    ImGui::Begin("Log");
    for (const auto& log : logs)
    {
        switch (log.type)
        {
        case Log::Type::Error:
            ImGui::TextColored(ImColor(255, 0, 0, 255), "%s", log.msg.c_str());
            break;
        case Log::Type::Warning:
            ImGui::TextColored(ImColor(255, 255, 0, 255), "%s", log.msg.c_str());
            break;
        case Log::Type::Debug:
            ImGui::TextColored(ImColor(150, 150, 150, 255), "%s", log.msg.c_str());
            break;
        default:
            break;
        }
    }
    ImGui::End();

}
void Editor::OnEvent(SDL_Event& event)
{
    switch (event.type)
    {
    case SDL_WINDOWEVENT:
    {
        switch (event.window.event)
        {
        case SDL_WINDOWEVENT_FOCUS_GAINED:
        {
            //TODO check if data content changed
            break;
        }
        default:break;
        }
        break;
    }
    case SDL_KEYDOWN:
    {
        const Uint8* state = SDL_GetKeyboardState(nullptr);
        switch (event.key.keysym.sym)
        {
        case SDLK_s:
        {
            
            if (state[SDL_SCANCODE_LCTRL]) 
            {
                SaveProject();
            }
            break;
        }
        case SDLK_o:
        {
            if (state[SDL_SCANCODE_LCTRL])
            {
                OpenFileBrowserDialog(FileBrowserMode::OPEN_FILE);
            }
            break;
        }
        default:break;
        }

        break;
    }
    default:break;
    }

}
void Editor::LoadFileIntoEditor(std::string_view path)
{

    EditorSystem* editorSystem = FindEditorSystem(path);
    if (editorSystem == nullptr)
    {
        LogError(fmt::format("Could not find appropriated editor system for file: {}", path));
        return;
    }
    const auto dstPath = fmt::format("{}{}{}",
                                     ResourceManager::dataFolder,
                                     editorSystem->GetSubFolder(),
                                     GetFilename(path));
    if (CopyFile(path, dstPath))
    {
        resourceManager_.AddResource(dstPath);
    }
}

void Editor::DrawEditorContent()
{
    ImGui::Begin("Editor Content");

    bool open = ImGui::TreeNode("Shaders");
    if (ImGui::BeginPopupContextItem())
    {
        if(ImGui::Button("Create New Shader"))
        {
            OpenFileBrowserDialog(FileBrowserMode::CREATE_NEW_SHADER);
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if(open)
    {
        if (editorSystems_[static_cast<int>(EditorType::SHADER)]->DrawContentList(currentFocusedSystem_ != EditorType::SHADER))
        {
            currentFocusedSystem_ = EditorType::SHADER;
        }
        ImGui::TreePop();
    }

    open = ImGui::TreeNode("Pipelines");
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::Button("Create New Pipeline"))
        {
            OpenFileBrowserDialog(FileBrowserMode::CREATE_NEW_PIPELINE);
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if(open)
    {
        if (editorSystems_[static_cast<int>(EditorType::PIPELINE)]->DrawContentList(currentFocusedSystem_ != EditorType::PIPELINE))
        {
            currentFocusedSystem_ = EditorType::PIPELINE;
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Materials"))
    {
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Textures"))
    {
        ImGui::TreePop();
    }
    ImGui::End();
}

void Editor::AddResource(const Resource& resource)
{
    EditorSystem* editorSystem = FindEditorSystem(resource.path);
    if (editorSystem == nullptr)
        return;
    editorSystem->AddResource(resource);
}

void Editor::RemoveResource(const Resource& resource)
{
    EditorSystem* editorSystem = FindEditorSystem(resource.path);
    if (editorSystem == nullptr)
        return;
    editorSystem->RemoveResource(resource);
}

void Editor::UpdateResource(const Resource& resource)
{
    EditorSystem* editorSystem = FindEditorSystem(resource.path);
    if (editorSystem == nullptr)
        return;
    editorSystem->UpdateResource(resource);
}

EditorSystem* Editor::GetEditorSystem(EditorType type) const
{
    return editorSystems_[static_cast<int>(type)].get();
}

EditorSystem* Editor::FindEditorSystem(std::string_view path) const
{
    EditorSystem* editorSystem = nullptr;
    const auto extension = GetFileExtension(path);
    for (auto& editorSystemTmp : editorSystems_)
    {
        if (!editorSystemTmp)
            continue;
        if (!editorSystemTmp->CheckExtensions(extension))
            continue;
        editorSystem = editorSystemTmp.get();
        break;
    }
    return editorSystem;
}
void Editor::OpenFileBrowserDialog(Editor::FileBrowserMode mode)
{
    switch(mode)
    {
    case FileBrowserMode::OPEN_FILE:
    {
        fileBrowserMode_ = FileBrowserMode::OPEN_FILE;
        fileDialog_ = ImGui::FileBrowser();
        fileDialog_.Open();
        break;
    }
    case FileBrowserMode::CREATE_NEW_SHADER:
    {
        fileBrowserMode_ = FileBrowserMode::CREATE_NEW_SHADER;
        fileDialog_ = ImGui::FileBrowser(ImGuiFileBrowserFlags_EnterNewFilename|ImGuiFileBrowserFlags_CreateNewDir);
        const auto shaderPath = fmt::format("{}{}", ResourceManager::dataFolder, editorSystems_[(int)EditorType::SHADER]->GetSubFolder());
        fileDialog_.SetPwd(shaderPath);
        fileDialog_.SetTypeFilters({".vert", ".frag", ".comp"});
        fileDialog_.Open();
        break;
    }
        case FileBrowserMode::CREATE_NEW_PIPELINE:
    {
        fileBrowserMode_ = FileBrowserMode::CREATE_NEW_PIPELINE;
        fileDialog_ = ImGui::FileBrowser(ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CreateNewDir);
        const auto pipelinePath = fmt::format("{}{}", ResourceManager::dataFolder, editorSystems_[static_cast<int>(EditorType::PIPELINE)]->GetSubFolder());
        fileDialog_.SetPwd(pipelinePath);
        fileDialog_.SetTypeFilters({ ".pipe" });
        fileDialog_.Open();
        break;
    }
    default:
        break;
    }
}
}