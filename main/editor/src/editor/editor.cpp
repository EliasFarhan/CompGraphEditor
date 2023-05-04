#include "editor.h"
#include "shader_editor.h"
#include "utils/log.h"
#include <imgui.h>
#include <imgui_stdlib.h>
#include <SDL.h>
#include <pybind11/embed.h>
#include <fmt/format.h>
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


#include <filesystem>
namespace fs = std::filesystem;

namespace editor
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
    
    resourceManager_.RegisterResourceChange(this);
    py::initialize_interpreter();
    const auto& filesystem = core::FilesystemLocator::get();
    if (!filesystem.IsDirectory(ResourceManager::dataFolder))
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


void Editor::OpenMenuCreateNewFile(EditorType editorType)
{
    ImGui::OpenPopup("Create New File");
    currentCreateFileSystem_ = editorType;
    currentExtensionCreateFileIndex_ = 0;
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
    const auto& filesystem = core::FilesystemLocator::get();
    switch (type)
    {
    case EditorType::SHADER:
    {
        filesystem.WriteString(path, "#version 310 es\nprecision highp float;\nvoid main() {}");
        resourceManager_.AddResource(path);
        break;
    }
    case EditorType::PIPELINE:
    {
        core::pb::Pipeline emptyPipeline;
        emptyPipeline.set_depth_mask(true);
        emptyPipeline.set_depth_compare_op(core::pb::Pipeline_DepthCompareOp_LESS);

        emptyPipeline.set_stencil_depth_fail(core::pb::Pipeline_StencilOp_KEEP);
        emptyPipeline.set_stencil_depth_pass(core::pb::Pipeline_StencilOp_KEEP);
        emptyPipeline.set_stencil_source_fail(core::pb::Pipeline_StencilOp_KEEP);
        emptyPipeline.set_stencil_mask(0xFF);

        emptyPipeline.set_blending_source_factor(core::pb::Pipeline_BlendFunc_SRC_ALPHA);
        emptyPipeline.set_blending_destination_factor(core::pb::Pipeline_BlendFunc_ONE_MINUS_SRC_ALPHA);

        filesystem.WriteString(path, emptyPipeline.SerializeAsString());
        resourceManager_.AddResource(path);
        break;
    }
    case EditorType::MESH: 
    {
        core::pb::Mesh emptyMesh;
        auto* scale = emptyMesh.mutable_scale();
        scale->set_x(1.0f);
        scale->set_y(1.0f);
        scale->set_z(1.0f);
        filesystem.WriteString(path, emptyMesh.SerializeAsString());
        resourceManager_.AddResource(path);
        break;
    }
    case EditorType::MATERIAL: 
    {
        const core::pb::Material emptyMaterial;
        filesystem.WriteString(path, emptyMaterial.SerializeAsString());
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
        core::pb::Scene emptyScene;
        emptyScene.set_name(GetFilename(path, false));
        CreateNewDirectory(GetFolder(path));
        filesystem.WriteString(path, emptyScene.SerializeAsString());
        resourceManager_.AddResource(path);
        sceneEditor->SetCurrentScene();
        for (const auto& editorSystem : editorSystems_)
        {
            if (!editorSystem && editorSystem->GetEditorType() != EditorType::SCENE)
                continue;
            const auto subFolder = fmt::format("{}{}/{}", 
                ResourceManager::dataFolder, 
                sceneEditor->GetCurrentSceneInfo()->info.name(), 
                editorSystem->GetSubFolder());
            if (!filesystem.IsDirectory(subFolder))
                CreateNewDirectory(subFolder);
            if(editorSystem->GetEditorType() == EditorType::SCRIPT)
            {
                CopyFileFromTo("scripts/neko2.py", fmt::format("{}/neko2.py", subFolder), true);
            }
            editorSystem->ReloadId();
        }
        break;
    }
    case EditorType::RENDER_PASS:
    {
        const core::pb::RenderPass emptyRenderPass;
        filesystem.WriteString(path, emptyRenderPass.SerializeAsString());
        resourceManager_.AddResource(path);
        break;
    }
    case EditorType::COMMAND:
    {
        core::pb::DrawCommand emptyDrawCommand;
        filesystem.WriteString(path, emptyDrawCommand.SerializeAsString());
        resourceManager_.AddResource(path);
        break;
    }
    case EditorType::FRAMEBUFFER:
    {
        core::pb::FrameBuffer emptyFramebuffer;
        filesystem.WriteString(path, emptyFramebuffer.SerializeAsString());
        resourceManager_.AddResource(path);
        break;
    }
    case EditorType::SCRIPT: 
    {
        filesystem.WriteString(path, "from neko2 import *\n");
        resourceManager_.AddResource(path);
        break;
    }
    case EditorType::TEXTURE:
    {
        if(GetFileExtension(path) == ".cube")
        {
            core::pb::Cubemap emptyCubemap;
            filesystem.WriteString(path, emptyCubemap.SerializeAsString());
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
        const auto& filesystem = core::FilesystemLocator::get();
        auto* editorSystem = editorSystems_[static_cast<int>(currentCreateFileSystem_)].get();
        const auto extensions = editorSystem->GetExtensions();
        auto* sceneEditor = static_cast<SceneEditor*>(GetEditorSystem(EditorType::SCENE));

        ImGui::InputText("Filename", &newCreateFilename_);
        std::string actualFilename = newCreateFilename_;
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
        if (!editorSystem->CheckExtensions(GetFileExtension(actualFilename)))
        {
            if (extensions.size() > 1)
            {
                if (ImGui::BeginCombo("Extension", extensions[currentExtensionCreateFileIndex_].data()))
                {
                    for(std::size_t i = 0; i < extensions.size();i++)
                    {
                        if(ImGui::Selectable(extensions[i].data(), i == currentExtensionCreateFileIndex_))
                        {
                            currentExtensionCreateFileIndex_ = i;
                        }
                    }
                    ImGui::EndCombo();
                }
                actualFilename += extensions[currentExtensionCreateFileIndex_];
            }
            else
            {
                actualFilename += extensions[0];
            }
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
        const auto path = fmt::format("{}{}/{}{}", 
            ResourceManager::dataFolder,
            sceneInfo ? sceneInfo->info.name() : newCreateFilename_,
            editorSystem->GetSubFolder(), 
            actualFilename);
        static bool isVulkanScene = false;
        if(currentCreateFileSystem_ == EditorType::SCENE)
        {
            ImGui::Checkbox("Vulkan", &isVulkanScene);
        }
        if (!filesystem.FileExists(path))
        {
            ImGui::Text("%s", path.c_str());
            if (ImGui::Button("Confirm"))
            {
                CreateNewFile(path, currentCreateFileSystem_);
                if (currentCreateFileSystem_ == EditorType::SCENE)
                {
                    sceneEditor->GetCurrentSceneInfo()->info.mutable_scene()->set_is_vulkan(isVulkanScene);
                }
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
        const auto path = fs::relative(fileDialog_.GetSelected()).string();

        LogDebug(fmt::format("Selected filename: {}", path));
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
    case SDL_QUIT:
        SaveProject();
        break;
    case SDL_WINDOWEVENT:
    {
        switch (event.window.event)
        {
        case SDL_WINDOWEVENT_FOCUS_GAINED:
        {
            const auto* sceneInfo = GetSceneEditor()->GetCurrentSceneInfo();
            if (sceneInfo == nullptr)
                break;
            resourceManager_.CheckDataFolder(sceneInfo->info.resources());
            RecursiveSceneFileReload();
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
        case SDLK_n:
        {
                //TODO create a new scene prompt
            break;
        }
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
        LogError(fmt::format("Could not find appropriated editor system for file: {}", path));
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
        CopyFileFromTo("scripts/neko2.py", fmt::format("data/{}/scripts/neko2.py", sceneEditor->GetCurrentSceneInfo()->info.name()));
        for(auto& tmp: editorSystems_)
        {
            if(tmp)
            {
                tmp->ReloadId();
            }
        }

        RecursiveSceneFileReload();
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
                if (std::ranges::none_of(sceneInfo->info.resources(),
                    [&folderContentPath, &filePath](const auto& path)
                    {
                        if (!fs::exists(path))
                            return false;
                        return fs::equivalent(filePath, path);
                    }))
                {
                    sceneInfo->info.add_resources(filePath);
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

    open = ImGui::TreeNode("Draw Commands");
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::Button("Create New Draw Command"))
        {
            OpenMenuCreateNewFile(EditorType::COMMAND);
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
            OpenMenuCreateNewFile(EditorType::TEXTURE);
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
