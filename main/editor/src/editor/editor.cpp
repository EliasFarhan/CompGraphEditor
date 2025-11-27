#include "editor.h"
#include "shader_editor.h"
#include "utils/log.h"
#include <imgui.h>
#include <imgui_stdlib.h>
#include <SDL3/SDL.h>
#include <pybind11/embed.h>
#include <format>
#include "engine/filesystem.h"
#include "editor_filesystem.h"
#include "py_interface.h"

#include "imnodes.h"
#include "pipeline_editor.h"
#include "material_editor.h"
#include "mesh_editor.h"
#include "render_pass_editor.h"
#include "command_editor.h"
#include "model_editor.h"
#include "scene_editor.h"
#include "script_editor.h"
#include "texture_editor.h"
#include "framebuffer_editor.h"
#include "pbr_utils.h"
#include "buffer_editor.h"


#include <filesystem>

#include "utils/fb_file.h"
namespace fs = std::filesystem;

namespace novus::editor
{

void Editor::Begin()
{
    ImNodes::CreateContext();

    editorSystems_.resize(static_cast<std::size_t>(EditorType::LENGTH));
    editorSystems_[static_cast<std::size_t>(EditorType::SHADER)] = std::make_unique<ShaderEditor>();
    editorSystems_[static_cast<std::size_t>(EditorType::PIPELINE)] = std::make_unique<PipelineEditor>();
    editorSystems_[static_cast<std::size_t>(EditorType::MATERIAL)] = std::make_unique<MaterialEditor>();
    editorSystems_[static_cast<std::size_t>(EditorType::MESH)] = std::make_unique<MeshEditor>();
    editorSystems_[static_cast<std::size_t>(EditorType::COMMAND)] = std::make_unique<CommandEditor>();
    editorSystems_[static_cast<std::size_t>(EditorType::SCRIPT)] = std::make_unique<ScriptEditor>();
    editorSystems_[static_cast<std::size_t>(EditorType::TEXTURE)] = std::make_unique<TextureEditor>();
    editorSystems_[static_cast<std::size_t>(EditorType::MODEL)] = std::make_unique<ModelEditor>();
    editorSystems_[static_cast<std::size_t>(EditorType::FRAMEBUFFER)] = std::make_unique<FramebufferEditor>();
    editorSystems_[static_cast<std::size_t>(EditorType::RENDER_PASS)] = std::make_unique<RenderPassEditor>();
    editorSystems_[static_cast<std::size_t>(EditorType::SCENE)] = std::make_unique<SceneEditor>();
    editorSystems_[static_cast<std::size_t>(EditorType::BUFFER)] = std::make_unique<BufferEditor>();
    resourceManager_.RegisterResourceChange(this);
    py::initialize_interpreter();
    if (!core::IsDirectory(ResourceManager::dataFolder))
    {
        CreateNewDirectory(ResourceManager::dataFolder);
    }

}

Editor::Editor()
{
    instance_ = this;
}

void Editor::OnGui()
{
    const auto windowSize = core::GetWindowSize();

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(windowSize.x * 0.2f, windowSize.y), ImGuiCond_FirstUseEver);
    DrawEditorContent();

    
    ImGui::SetNextWindowPos(ImVec2(windowSize.x * 0.2f, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(windowSize.x * 0.6f, windowSize.y), ImGuiCond_FirstUseEver);
    //ImGui::Begin("Center View", nullptr, ImGuiWindowFlags_NoTitleBar);

    DrawCenterView();
    //ImGui::End();
    
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


void Editor::OpenMenuCreateNewFile(EditorType editorType, std::string_view extension)
{
    ImGui::OpenPopup("Create New File");
    currentCreateFileSystem_ = editorType;
    currentExtensionCreateFileIndex_ = 0;
    newCreateExtension_ = extension;
    newCreateFilename_.clear();
}

void Editor::SaveProject() const
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
    bool createNewFile = false;
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("New Scene"))
            {
                createNewFile = true;
            }
            if (ImGui::MenuItem("Open Scene"))
            {
                OpenFileBrowserDialog(editorSystems_[static_cast<int>(EditorType::SCENE)]->GetExtensions());
            }
            if(ImGui::MenuItem("Save"))
            {
                SaveProject();
            }
            if(ImGui::MenuItem("Export & Play"))
            {
                auto* sceneEditor = GetSceneEditor();
                auto* sceneInfo = sceneEditor->GetCurrentSceneInfo();
                if(sceneInfo != nullptr)
                {
                    sceneEditor->ExportAndPlayScene();
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Window"))
        {
            //TODO put editor list
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    if(createNewFile)
    {
        OpenMenuCreateNewFile(EditorType::SCENE);
    }
    if(UpdateCreateNewFile())
    {
        
    }
}

void Editor::CreateNewFile(std::string_view path, EditorType type)
{
    switch (type)
    {
    case EditorType::SHADER:
    {
        core::WriteString(path, "#version 310 es\nprecision highp float;\nvoid main() {}");
        resourceManager_.AddResource(path);
        break;
    }
    case EditorType::PIPELINE:
    {
        novus::renderer::GraphicsPipelineT emptyPipeline;
        auto pipelineInfo = std::make_unique<novus::internal::GraphicsPipelineInfoT>();
        auto depthStencilState = std::make_unique<novus::internal::DepthStencilStateT>();
        depthStencilState->write_mask = 0xFF;
        depthStencilState->compare_mask = 0xFF;
        depthStencilState->compare_op = internal::CompareOp_COMPAREOP_LESS;

        auto frontStencilState = std::make_unique<internal::StencilOpStateT>();
        frontStencilState->compare_op = internal::StencilOp_STENCILOP_KEEP;
        frontStencilState->depth_fail_op = internal::StencilOp_STENCILOP_KEEP;
        frontStencilState->fail_op = internal::StencilOp_STENCILOP_KEEP;
        frontStencilState->pass_op = internal::StencilOp_STENCILOP_KEEP;
        auto backStencilState = std::make_unique<internal::StencilOpStateT>();
        *backStencilState = *frontStencilState;
        depthStencilState->back_stencil_state = std::move(frontStencilState);
        depthStencilState->front_stencil_state = std::move(backStencilState);

        pipelineInfo->depth_stencil_state = std::move(depthStencilState);

        emptyPipeline.info = std::move(pipelineInfo);
        core::WriteFlatbufferToFile<renderer::GraphicsPipelineT, renderer::GraphicsPipeline>(emptyPipeline, path);
        resourceManager_.AddResource(path);
        break;
    }
    case EditorType::MESH: 
    {
        renderer::MeshT emptyMesh;
        emptyMesh.scale = {1,1,1};
        core::WriteFlatbufferToFile<renderer::MeshT, renderer::Mesh>(emptyMesh, path);
        resourceManager_.AddResource(path);
        break;
    }
    case EditorType::MATERIAL: 
    {
        renderer::MaterialT emptyMaterial;
        core::WriteFlatbufferToFile<renderer::MaterialT, renderer::Material>(emptyMaterial, path);
        resourceManager_.AddResource(path);
        break;
    }
    case EditorType::SCENE:
    {
        auto* sceneEditor = GetSceneEditor();
        if(sceneEditor->GetCurrentSceneInfo() != nullptr)
        {
            SaveProject();
            for (auto& tmp : editorSystems_)
            {
                if (tmp)
                {
                    tmp->Clear();
                }
            }
            resourceManager_.Clear();
        }
        renderer::SceneT emptyScene;
        emptyScene.name = (GetFilename(path, false));
        CreateNewDirectory(GetFolder(path));
        core::WriteFlatbufferToFile<renderer::SceneT, renderer::Scene>(emptyScene, path);
        resourceManager_.AddResource(path);
        sceneEditor->SetCurrentScene();
        for (const auto& editorSystem : editorSystems_)
        {
            if (!editorSystem && editorSystem->GetEditorType() != EditorType::SCENE)
                continue;
            std::string subFolder { std::format("{}{}/{}",
                ResourceManager::dataFolder.data(),
                sceneEditor->GetCurrentSceneInfo()->name,
                editorSystem->GetSubFolder())};
            if (!core::IsDirectory(subFolder.c_str()))
            {
                CreateNewDirectory(subFolder);
            }
            if(editorSystem->GetEditorType() == EditorType::SCRIPT)
            {
                //CopyFileFromTo("scripts/neko2.py", std::format("{}/neko2.py", subFolder.c_str()), true);
            }
            editorSystem->ReloadId();
        }
        break;
    }
    case EditorType::RENDER_PASS:
    {
        EditorRenderPassInfoT emptyRenderPassInfo;
        core::WriteFlatbufferToFile<EditorRenderPassInfoT, EditorRenderPassInfo>(emptyRenderPassInfo, path);
        resourceManager_.AddResource(path);
        break;
    }
    case EditorType::COMMAND:
    {
        renderer::DrawCommandT emptyDrawCommand;
        core::WriteFlatbufferToFile<renderer::DrawCommandT, renderer::DrawCommand>(emptyDrawCommand, path);
        resourceManager_.AddResource(path);
        break;
    }
    case EditorType::FRAMEBUFFER:
    {
        renderer::FramebufferT emptyFramebuffer;
        core::WriteFlatbufferToFile<renderer::FramebufferT, renderer::Framebuffer>(emptyFramebuffer, path);
        resourceManager_.AddResource(path);
        break;
    }
    case EditorType::BUFFER:
    {
        renderer::StorageBufferT emptyBuffer{};
        core::WriteFlatbufferToFile<renderer::StorageBufferT, renderer::StorageBuffer>(emptyBuffer, path);
        resourceManager_.AddResource(path);
        break;
    }
    case EditorType::SCRIPT: 
    {
        //TODO generate basic cpp wasm script
        core::WriteString(path, "from neko2 import *\n");
        resourceManager_.AddResource(path);
        break;
    }
    case EditorType::TEXTURE:
    {
        if(GetFileExtension(path) == ".cube")
        {
            renderer::CubemapT emptyCubemap;
            core::WriteFlatbufferToFile<renderer::CubemapT, renderer::Cubemap>(emptyCubemap, path);
            resourceManager_.AddResource(path);
        }
        break;
    }
    default: 
        break;
    }

    auto* sceneEditor = GetSceneEditor();
    if(type != EditorType::SCENE)
    {
        sceneEditor->AddResource(*resourceManager_.GetResource(resourceManager_.FindResourceByPath(path)));
    }
}

bool Editor::UpdateCreateNewFile()
{
    if (ImGui::BeginPopupModal("Create New File"))
    {
        auto* editorSystem = editorSystems_[static_cast<int>(currentCreateFileSystem_)].get();
        const auto extensions = editorSystem->GetExtensions();
        auto* sceneEditor = static_cast<SceneEditor*>(GetEditorSystem(EditorType::SCENE));

        ImGui::InputText("Filename", &newCreateFilename_);
        std::string actualFilename {newCreateFilename_};
        if(actualFilename.empty())
        {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Empty filename");
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
            return false;
        }
        if (newCreateExtension_.empty())
        {
            if (!editorSystem->CheckExtensions(GetFileExtension(actualFilename)))
            {
                if (extensions.size() > 1)
                {
                    if (ImGui::BeginCombo("Extension", extensions[currentExtensionCreateFileIndex_].data()))
                    {
                        for (std::size_t i = 0; i < extensions.size(); i++)
                        {
                            if (ImGui::Selectable(extensions[i].data(), i == currentExtensionCreateFileIndex_))
                            {
                                currentExtensionCreateFileIndex_ = i;
                            }
                        }
                        ImGui::EndCombo();
                    }
                    actualFilename += extensions[currentExtensionCreateFileIndex_].data();
                }
                else
                {
                    actualFilename += extensions[0].data();
                }
            }
        }
        else
        {
            actualFilename += newCreateExtension_.data();
        }
        const auto* sceneInfo = sceneEditor->GetCurrentSceneInfo();
        if(sceneInfo == nullptr)
        {
            if (currentCreateFileSystem_ != EditorType::SCENE)
            {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Create or open a new scene before anything!");
                ImGui::EndPopup();
                return false;
            }
        }
        std::string path;
        static bool isVulkanScene = false;
        
        if(currentCreateFileSystem_ == EditorType::SCENE)
        {
            path = std::format("{}/{}/{}",
                ResourceManager::dataFolder.data(),
                newCreateFilename_,
                actualFilename.c_str());
            ImGui::Checkbox("Vulkan", &isVulkanScene);
        }
        else
        {
            path = std::format("{}{}/{}{}",
                ResourceManager::dataFolder.data(),
                sceneInfo ? sceneInfo->name : newCreateFilename_,
                editorSystem->GetSubFolder(),
                actualFilename.c_str());
        }
        if (!core::FileExists(path))
        {
            ImGui::Text("%s", path.c_str());
            if (ImGui::Button("Confirm"))
            {
                CreateNewFile(path, currentCreateFileSystem_);
                ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
                return true;
            }
        }
        else
        {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "File Exists: %s", path.c_str());
        }
        if(ImGui::Button("Close"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    return false;
}

void Editor::DrawCenterView()
{
    ImGui::Begin("Center View");
    DrawMenuBar();
    if (currentFocusedSystem_ != EditorType::LENGTH)
    {
        auto* editorSystem = editorSystems_[static_cast<int>(currentFocusedSystem_)].get();
        if (editorSystem)
        {
            editorSystem->DrawCenterView();
        }
    }
    ImGui::End();
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
        const auto path = fileDialog_.GetSelected().string();

        LogDebug(std::format("Selected filename: {}", path.c_str()));
        LoadFileIntoEditor(path);
        fileDialog_.ClearSelected();
    }
    
}
void Editor::DrawLogWindow()
{
    const auto& logs = core::GetLogs();
    ImGui::Begin("Log");
    for (const auto& log : logs)
    {
        switch (log.type)
        {
        case core::Log::Type::Error:
            ImGui::TextColored(ImColor(255, 0, 0, 255), "%s", log.msg.c_str());
            break;
        case core::Log::Type::Warning:
            ImGui::TextColored(ImColor(255, 255, 0, 255), "%s", log.msg.c_str());
            break;
        case core::Log::Type::Debug:
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
    case SDL_EVENT_QUIT:
        SaveProject();
        break;
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
    {
        const auto* sceneInfo = GetSceneEditor()->GetCurrentSceneInfo();
        if (sceneInfo == nullptr)
            break;
        resourceManager_.CheckDataFolder(sceneInfo->resources);
        RecursiveSceneFileReload();
        break;
    }
    case SDL_EVENT_KEY_DOWN:
    {
        const bool* state = SDL_GetKeyboardState(nullptr);
        switch (event.key.key)
        {
        case SDLK_E:
        {
            if (state[SDL_SCANCODE_LCTRL])
            {
                GetSceneEditor()->ExportAndPlayScene();
            }
                break;
        }
        case SDLK_N:
        {
            if (state[SDL_SCANCODE_LCTRL])
            {
                static constexpr std::array<std::string_view, 1> extensions = { ".scene" };
                OpenFileBrowserDialog(extensions);
            }
            break;
        }
        case SDLK_S:
        {
            
            if (state[SDL_SCANCODE_LCTRL]) 
            {
                SaveProject();
            }
            break;
        }
        case SDLK_O:
        {
            if (state[SDL_SCANCODE_LCTRL])
            {
                std::span<const std::string_view> extensions = {};
                auto* sceneEditor = GetSceneEditor();
                if(sceneEditor->GetCurrentSceneInfo() == nullptr)
                {
                    extensions = sceneEditor->GetExtensions();
                }
                OpenFileBrowserDialog(extensions);
            }
            break;
        }
        case SDLK_DELETE:
        {
            auto* editorSystem = editorSystems_[static_cast<int>(currentFocusedSystem_)].get();
            if(editorSystem != nullptr)
            {
                editorSystem->Delete();
            }
            break;
        }
        default:
            break;
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
        LogError(std::format("Could not find appropriated editor system for file: {}", path.data()));
        return;
    }
    auto* sceneEditor = GetSceneEditor();
    const bool isScene = sceneEditor->CheckExtensions(GetFileExtension(path));
    if(isScene)
    {
        //Opening a new scene means closing the current scene
        if(sceneEditor->GetCurrentSceneInfo() != nullptr)
        {
            SaveProject();
            for(auto& tmp : editorSystems_)
            {
                if(tmp)
                {
                    tmp->Clear();
                }
            }
            resourceManager_.Clear();
        }
    }
    else
    {
        if(sceneEditor->GetCurrentSceneInfo() == nullptr)
        {
            //No importing file if no current scene
            return;
        }
    }
    
    editorSystem->ImportResource(path);
    if(isScene)
    {
        //CopyFileFromTo("scripts/neko2.py",
          //      std::format("data/{}/scripts/neko2.py", sceneEditor->GetCurrentSceneInfo()->name));
        for(auto& tmp: editorSystems_)
        {
            if(tmp)
            {
                tmp->ReloadId();
            }
        }

        RecursiveSceneFileReload();
        for (const auto& editorSystem : editorSystems_)
        {
            if (!editorSystem && editorSystem->GetEditorType() != EditorType::SCENE)
                continue;
            const auto subFolder{ std::format("{}{}/{}",
                ResourceManager::dataFolder,
                sceneEditor->GetCurrentSceneInfo()->name,
                editorSystem->GetSubFolder())};
            if (!core::IsDirectory(subFolder))
                CreateNewDirectory(subFolder);
            if (editorSystem->GetEditorType() == EditorType::SCRIPT)
            {
                CopyFileFromTo("scripts/neko2.py", std::format("{}/neko2.py", subFolder), true);
            }
            editorSystem->ReloadId();
        }
    }

}

void Editor::RecursiveSceneFileReload()
{
    auto* sceneEditor = GetSceneEditor();
    auto* sceneInfo = sceneEditor->GetCurrentSceneInfo();
    if (sceneInfo == nullptr)
        return;
    const auto sceneFolder = GetFolder(sceneInfo->path);
    //recursive search file
    std::function<void(std::string_view)> recursiveSearch = [&recursiveSearch, &sceneInfo, this](std::string_view folder)
    {
        const fs::path folderPath = folder;
        for (const auto& entry : fs::directory_iterator(folderPath))
        {
            const auto& folderContentPath = entry.path();
            if (fs::is_directory(folderContentPath))
            {
                recursiveSearch(folderContentPath.string());
            }
            else
            {
                if(folderContentPath.extension() == ".scene" || folderContentPath.extension() == ".pkg")
                    continue;
                const auto filePath = folderContentPath.string();
                if (std::ranges::none_of(sceneInfo->resources,
                    [&filePath](const auto& path)
                    {
                        if (!fs::exists(path))
                            return false;
                        return fs::equivalent(filePath.c_str(), path);
                    }))
                {
                    sceneInfo->resources.push_back(filePath);
                    resourceManager_.AddResource(filePath);
                }
            }
        }
    };
    recursiveSearch(sceneFolder);
}

void Editor::DrawEditorContent()
{
    const bool hasScene = GetSceneEditor()->GetCurrentSceneInfo() != nullptr;

    ImGui::Begin("Editor Content");
    if (!hasScene)
    {
        ImGui::TextColored(ImVec4{ 1.0f, 0.0f, 0.0f, 1.0f }, "Create or open a new scene");
        ImGui::End();
        return;
    }
    bool open = ImGui::TreeNode("Shaders");
    if (ImGui::BeginPopupContextItem())
    {
        if(ImGui::Button("Create New Shader"))
        {
            OpenMenuCreateNewFile(EditorType::SHADER);
        }
        if(UpdateCreateNewFile())
        {
            ImGui::CloseCurrentPopup();
        }
        if(ImGui::Button("Import Shader"))
        {
            OpenFileBrowserDialog(editorSystems_[static_cast<int>(EditorType::SHADER)]->GetExtensions());
        }
        ImGui::EndPopup();
    }
    if(open)
    {
        if (editorSystems_[static_cast<int>(EditorType::SHADER)]
            ->DrawContentList(currentFocusedSystem_ != EditorType::SHADER))
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
            OpenMenuCreateNewFile(EditorType::PIPELINE);
        }
        if (UpdateCreateNewFile())
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if(open)
    {
        if (editorSystems_[static_cast<int>(EditorType::PIPELINE)]
            ->DrawContentList(currentFocusedSystem_ != EditorType::PIPELINE))
        {
            currentFocusedSystem_ = EditorType::PIPELINE;
        }
        ImGui::TreePop();
    }

    open = ImGui::TreeNode("Materials");
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::Button("Create New Material"))
        {
            OpenMenuCreateNewFile(EditorType::MATERIAL);
        }
        if(UpdateCreateNewFile())
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if(open)
    {
        if (editorSystems_[static_cast<int>(EditorType::MATERIAL)]
            ->DrawContentList(currentFocusedSystem_ != EditorType::MATERIAL))
        {
            currentFocusedSystem_ = EditorType::MATERIAL;
        }
        ImGui::TreePop();
    }

    open = ImGui::TreeNode("Meshes");
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::Button("Create New Mesh"))
        {
            OpenMenuCreateNewFile(EditorType::MESH);
        }
        if (UpdateCreateNewFile())
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if(open)
    {
        if (editorSystems_[static_cast<int>(EditorType::MESH)]
                ->DrawContentList(currentFocusedSystem_ != EditorType::MESH))
        {
            currentFocusedSystem_ = EditorType::MESH;
        }
        ImGui::TreePop();
    }

    open = ImGui::TreeNode("Commands");
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::Button("Create New Draw Command"))
        {
            OpenMenuCreateNewFile(EditorType::COMMAND, ".cmd");
        }
        if (ImGui::Button("Create New Compute Command"))
        {
            OpenMenuCreateNewFile(EditorType::COMMAND, ".compcmd");
        }
        if (UpdateCreateNewFile())
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if (open)
    {
        if (editorSystems_[static_cast<int>(EditorType::COMMAND)]
            ->DrawContentList(currentFocusedSystem_ != EditorType::COMMAND))
        {
            currentFocusedSystem_ = EditorType::COMMAND;
        }
        ImGui::TreePop();
    }

    open = ImGui::TreeNode("Render Passes");
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::Button("Create New Render Pass"))
        {
            OpenMenuCreateNewFile(EditorType::RENDER_PASS);
        }
        if (UpdateCreateNewFile())
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if (open)
    {
        if (editorSystems_[static_cast<int>(EditorType::RENDER_PASS)]
            ->DrawContentList(currentFocusedSystem_ != EditorType::RENDER_PASS))
        {
            currentFocusedSystem_ = EditorType::RENDER_PASS;
        }
        ImGui::TreePop();
    }

    
    
    open = ImGui::TreeNode("Scenes");
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::Button("Create New Scene"))
        {
            OpenMenuCreateNewFile(EditorType::SCENE);
        }
        if (UpdateCreateNewFile())
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if (open)
    {
        if (editorSystems_[static_cast<int>(EditorType::SCENE)]
            ->DrawContentList(currentFocusedSystem_ != EditorType::SCENE))
        {
            currentFocusedSystem_ = EditorType::SCENE;
        }
        ImGui::TreePop();
    }

    open = ImGui::TreeNode("Scripts");
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::Button("Create New Script"))
        {
            OpenMenuCreateNewFile(EditorType::SCRIPT);
        }

        if (UpdateCreateNewFile())
        {
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("Import Script"))
        {
            OpenFileBrowserDialog(editorSystems_[static_cast<int>(EditorType::SCRIPT)]->GetExtensions());
        }
        ImGui::EndPopup();
    }
    if (open)
    {
        if (editorSystems_[static_cast<int>(EditorType::SCRIPT)]
            ->DrawContentList(currentFocusedSystem_ != EditorType::SCRIPT))
        {
            currentFocusedSystem_ = EditorType::SCRIPT;
        }
        ImGui::TreePop();
    }

    open = ImGui::TreeNode("Textures");
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::Button("Create New Cubemap"))
        {
            OpenMenuCreateNewFile(EditorType::TEXTURE, ".cube");
        }

        if (UpdateCreateNewFile())
        {
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("Import Textures"))
        {
            OpenFileBrowserDialog(editorSystems_[static_cast<int>(EditorType::TEXTURE)]->GetExtensions());
        }

        if(ImGui::Button("Generate BRDF LUT"))
        {
            auto* textureEditor = dynamic_cast<TextureEditor*>(GetEditorSystem(EditorType::TEXTURE));
            GeneratePreComputeBrdfLUT();
        }
        ImGui::EndPopup();
    }
    if (open)
    {
        if (editorSystems_[static_cast<int>(EditorType::TEXTURE)]
            ->DrawContentList(currentFocusedSystem_ != EditorType::TEXTURE))
        {
            currentFocusedSystem_ = EditorType::TEXTURE;
        }
        ImGui::TreePop();
    }

    open = ImGui::TreeNode("Models");
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::Button("Import Models"))
        {
            OpenFileBrowserDialog(editorSystems_[static_cast<int>(EditorType::MODEL)]->GetExtensions());
        }
        ImGui::EndPopup();
    }
    if (open)
    {
        if (editorSystems_[static_cast<int>(EditorType::MODEL)]
            ->DrawContentList(currentFocusedSystem_ != EditorType::MODEL))
        {
            currentFocusedSystem_ = EditorType::MODEL;
        }
        ImGui::TreePop();
    }

    open = ImGui::TreeNode("Framebuffer");
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::Button("Create New Framebuffer"))
        {
            OpenMenuCreateNewFile(EditorType::FRAMEBUFFER);
        }

        if (UpdateCreateNewFile())
        {
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("Import Framebuffer"))
        {
            OpenFileBrowserDialog(editorSystems_[static_cast<int>(EditorType::FRAMEBUFFER)]->GetExtensions());
        }
        ImGui::EndPopup();
    }
    if (open)
    {
        if (editorSystems_[static_cast<int>(EditorType::FRAMEBUFFER)]
            ->DrawContentList(currentFocusedSystem_ != EditorType::FRAMEBUFFER))
        {
            currentFocusedSystem_ = EditorType::FRAMEBUFFER;
        }
        ImGui::TreePop();
    }

    open = ImGui::TreeNode("Buffer");
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::Button("Create New Buffer"))
        {
            OpenMenuCreateNewFile(EditorType::BUFFER);
        }

        if (UpdateCreateNewFile())
        {
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("Import Buffer"))
        {
            OpenFileBrowserDialog(editorSystems_[static_cast<int>(EditorType::BUFFER)]->GetExtensions());
        }
        ImGui::EndPopup();
    }
    if (open)
    {
        if (editorSystems_[static_cast<int>(EditorType::BUFFER)]
            ->DrawContentList(currentFocusedSystem_ != EditorType::BUFFER))
        {
            currentFocusedSystem_ = EditorType::BUFFER;
        }
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
    auto* sceneEditor = GetSceneEditor();
    if (sceneEditor->CheckExtensions(resource.extension))
    {
        sceneEditor->RemoveResource(resource);
    }
    editorSystem->RemoveResource(resource);
}

void Editor::UpdateExistingResource(const Resource& resource)
{
    EditorSystem* editorSystem = FindEditorSystem(resource.path);
    if (editorSystem == nullptr)
        return;
    editorSystem->UpdateExistingResource(resource);
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

ResourceManager& GetResourceManager()
{
    return Editor::GetInstance()->GetResourceManager();
}

void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) && ImGui::BeginTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void Editor::OpenFileBrowserDialog(std::span<const std::string_view> extensions)
{
    std::vector<std::string> args;
    args.reserve(extensions.size());
    for (auto extension : extensions)
    {
        args.emplace_back(extension.data());
    }
    fileDialog_ = ImGui::FileBrowser();
    fileDialog_.SetTypeFilters(args);
    fileDialog_.Open();
}
}
