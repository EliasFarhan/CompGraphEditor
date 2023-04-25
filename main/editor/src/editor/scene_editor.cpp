#include "scene_editor.h"
#include "engine/filesystem.h"
#include "utils/log.h"
#include "render_pass_editor.h"
#include "command_editor.h"

#include <imgui.h>
#include <fmt/format.h>
#include <fstream>

#include "editor.h"


#include <pybind11/pybind11.h>
#include <nlohmann/json.hpp>

#include "material_editor.h"
#include "mesh_editor.h"
#include "pipeline_editor.h"
#include "script_editor.h"
#include "shader_editor.h"
#include "texture_editor.h"
#include "model_editor.h"
#include "framebuffer_editor.h"
#include "gl/engine.h"

namespace py = pybind11;
using json = nlohmann::json;

namespace editor
{

void ExecutePlayer(std::string_view scenePkg)
{
    const bool isVulkan = GetSceneEditor()->IsVulkanScene();
    std::string baseName = isVulkan ? "vk_player" : "gl_player";
    std::string command; 
#ifdef _MSC_VER
    fs::path executable = fmt::format("{}.exe", baseName);
    if (fs::exists(executable))
    {
        command = fmt::format("{} {}", executable.generic_string(), scenePkg);
    }
    else
    {
#ifdef _DEBUG
        command = fmt::format(".\\Debug\\{} {}", executable.generic_string(), scenePkg);
#endif
    }
#else
    command = fmt::format("./{} {}", baseName, scenePkg);
#endif
    if(!isVulkan)
    {
        const auto glVersion = gl::GetGlVersion();
        command = fmt::format("{} --major={} --minor={} {}", command, glVersion.major, glVersion.minor, glVersion.es ? "--es" : "");
    }
    if(command.empty())
    {
        return;
    }
    system(command.c_str());
    LogDebug(fmt::format("Executing {}", command));
}

void SceneEditor::ImportResource(std::string_view path)
{
    //TODO Only copy scene if in another folder
    
    auto* editor = Editor::GetInstance();
    auto& resourceManager = editor->GetResourceManager();
    resourceManager.AddResource(path);
    
}

void SceneEditor::AddResource(const Resource& resource)
{
    if (!CheckExtensions(resource.extension))
    {
        //Adding a resource to the list
        auto* sceneInfo = GetCurrentSceneInfo();
        if (std::ranges::none_of(sceneInfo->info.resources(), [&resource](const auto& path) {return path == resource.path; }))
        {
            GetCurrentSceneInfo()->info.add_resources(resource.path);
        }
        return;
    }
    SceneInfo sceneInfo{};
    sceneInfo.resourceId = resource.resourceId;
    sceneInfo.filename = GetFilename(resource.path);

    const auto extension = GetFileExtension(resource.path);
    if (extension == ".scene")
    {
        const auto& fileSystem = core::FilesystemLocator::get();
        if (!fileSystem.IsRegularFile(resource.path))
        {
            LogWarning(fmt::format("Could not find scene file: {}", resource.path));
            return;
        }
        std::ifstream fileIn(resource.path, std::ios::binary);
        if (!sceneInfo.info.ParseFromIstream(&fileIn))
        {
            LogWarning(fmt::format("Could not open protobuf file: {}", resource.path));
            return;
        }
    }
    sceneInfo.path = resource.path;
    sceneInfos_.push_back(sceneInfo);

    currentIndex_ = 0;

    auto* editor = Editor::GetInstance();
    auto& resourceManager = editor->GetResourceManager();
    
    for(const auto& resourcePath: sceneInfo.info.resources())
    {
        if (resourceManager.FindResourceByPath(resourcePath) == INVALID_RESOURCE_ID)
        {
            resourceManager.AddResource(resourcePath);
        }
    }
    core::SetWindowName(fmt::format("Neko2 Editor - {}", sceneInfo.info.name()));
}

void SceneEditor::RemoveResource(const Resource& resource)
{
    if(!CheckExtensions(resource.extension))
    {
        //Removing a resource to the list
        auto* resources = GetCurrentSceneInfo()->info.mutable_resources();
        for(int i = 0; i < resources->size(); i++)
        {
            if(resources->Get(i) == resource.path)
            {
                resources->erase(resources->begin() + i);
                return;
            }
        }
        return;
    }
    for(auto& sceneInfo : sceneInfos_)
    {
        if(resource.resourceId == sceneInfo.renderPassId)
        {
            sceneInfo.renderPassId = INVALID_RESOURCE_ID;
            sceneInfo.info.clear_render_pass_path();
        }
    }

    const auto it = std::ranges::find_if(sceneInfos_, [&resource](const auto& renderPass)
        {
            return resource.resourceId == renderPass.resourceId;
        });
    if (it != sceneInfos_.end())
    {
        sceneInfos_.erase(it);
    }
}

void SceneEditor::UpdateExistingResource(const Resource& resource)
{
}


void SceneEditor::DrawInspector()
{
    if (currentIndex_ >= sceneInfos_.size())
    {
        return;
    }
    auto* editor = Editor::GetInstance();
    const auto& resourceManager = editor->GetResourceManager();
    auto* renderPassEditor = dynamic_cast<RenderPassEditor*>(editor->GetEditorSystem(EditorType::RENDER_PASS));
    auto* scriptEditor = dynamic_cast<ScriptEditor*>(editor->GetEditorSystem(EditorType::SCRIPT));
    auto* modelEditor = dynamic_cast<ModelEditor*>(editor->GetEditorSystem(EditorType::MODEL));
    auto& currentScene = sceneInfos_[currentIndex_];


    const auto& renderPasses = renderPassEditor->GetRenderPasses();
    const auto* renderPassInfo = renderPassEditor->GetRenderPass(currentScene.renderPassId);
    if(ImGui::BeginCombo("Render Pass", renderPassInfo?renderPassInfo->filename.c_str():"Empty Render Pass"))
    {
        for(auto& renderPass : renderPasses)
        {
            if(ImGui::Selectable(renderPass.filename.c_str(), renderPass.resourceId == currentScene.renderPassId))
            {
                currentScene.renderPassId = renderPass.resourceId;
                currentScene.info.set_render_pass_path(renderPass.path);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::Separator();
    ImGui::Text("Scripts");
    
    std::vector<int> removedScripts;
    for (int i = 0; i < currentScene.info.py_system_paths_size(); i++)
    {
        const auto& pySystemPath = currentScene.info.py_system_paths(i);
        std::string name = fmt::format("Script: {}", i);
        const ScriptInfo* scriptInfo = nullptr;
        ResourceId pySystemId = INVALID_RESOURCE_ID;
        if (!pySystemPath.empty())
        {
            pySystemId = resourceManager.FindResourceByPath(pySystemPath);
            if (pySystemId == INVALID_RESOURCE_ID)
            {
                currentScene.info.mutable_py_system_paths(i)->clear();
            }
            else
            {
                scriptInfo = scriptEditor->GetScriptInfo(pySystemId);
                name = fmt::format("Script: {}.{}", scriptInfo->info.module(), scriptInfo->info.class_());
            }
        }
        bool visible = true;
        if (ImGui::CollapsingHeader(name.c_str(), &visible))
        {
            const auto& pySystems = scriptEditor->GetScriptInfos();
            const auto scriptsId = fmt::format("script {} combo", i);
            ImGui::PushID(scriptsId.c_str());
            if (ImGui::BeginCombo("Available Scripts", scriptInfo ? scriptInfo->info.class_().c_str() : "Missing script"))
            {
                for (auto& pySystemInfo : pySystems)
                {
                    const auto selectedName = fmt::format("{}.{}", pySystemInfo.info.module(), pySystemInfo.info.class_());
                    if (ImGui::Selectable(selectedName.c_str(), pySystemId == pySystemInfo.resourceId))
                    {
                        *currentScene.info.mutable_py_system_paths(i) = pySystemInfo.info.path();
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopID();
            
        }
        if (!visible)
        {
            removedScripts.push_back(i);
        }
        
    }
    std::ranges::reverse(removedScripts);
    for(const auto index : removedScripts)
    {
        currentScene.info.mutable_py_system_paths()->DeleteSubrange(index, 1);
    }
    if(ImGui::Button("Add Script"))
    {
        currentScene.info.add_py_system_paths();
    }
}

bool SceneEditor::DrawContentList(bool unfocus)
{
    bool wasFocused = false;
    for (std::size_t i = 0; i < sceneInfos_.size(); i++)
    {
        const auto& sceneInfo = sceneInfos_[i];
        const auto selected = ImGui::Selectable(sceneInfo.filename.data(), unfocus);

        if (selected)
        {

            currentIndex_ = i;
            wasFocused = true;

        }
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::Button("Export Scene & Play"))
            {
                currentIndex_ = i;
                wasFocused = true;
                if(ExportAndPlayScene())
                {
                    LogDebug("Successfully export scene!");
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
    return wasFocused;
}

std::string_view SceneEditor::GetSubFolder()
{
    //No subfolder for scenes, as each scene has their own data folder
    return "";
}

EditorType SceneEditor::GetEditorType()
{
    return EditorType::SCENE;
}

void SceneEditor::Save()
{
    for (auto& sceneInfo : sceneInfos_)
    {
        std::ofstream fileOut(sceneInfo.path, std::ios::binary);
        if (!sceneInfo.info.SerializeToOstream(&fileOut))
        {
            LogWarning(fmt::format("Could not save scene at: {}", sceneInfo.path));
        }
    }
}

bool SceneEditor::ExportAndPlayScene() const
{
    if (currentIndex_ >= sceneInfos_.size())
    {
        LogWarning("Could not export no scene selected");
        return false;
    }
    auto* editor = Editor::GetInstance();
    const auto& resourceManager = editor->GetResourceManager();
    const auto* renderPassEditor = dynamic_cast<RenderPassEditor*>(editor->GetEditorSystem(EditorType::RENDER_PASS));
    auto* commandEditor = dynamic_cast<CommandEditor*>(editor->GetEditorSystem(EditorType::COMMAND));
    auto* materialEditor = dynamic_cast<MaterialEditor*>(editor->GetEditorSystem(EditorType::MATERIAL));
    const auto* pipelineEditor = dynamic_cast<PipelineEditor*>(editor->GetEditorSystem(EditorType::PIPELINE));
    const auto* shaderEditor = dynamic_cast<ShaderEditor*>(editor->GetEditorSystem(EditorType::SHADER));
    auto* meshEditor = dynamic_cast<MeshEditor*>(editor->GetEditorSystem(EditorType::MESH));
    auto* modelEditor = dynamic_cast<ModelEditor*>(editor->GetEditorSystem(EditorType::MODEL));
    const auto* scriptEditor = dynamic_cast<ScriptEditor*>(editor->GetEditorSystem(EditorType::SCRIPT));
    auto* textureEditor = dynamic_cast<TextureEditor*>(editor->GetEditorSystem(EditorType::TEXTURE));
    auto* framebufferEditor = dynamic_cast<FramebufferEditor*>(editor->GetEditorSystem(EditorType::FRAMEBUFFER));
    //TODO reload all editors to get all correct resourceId
    commandEditor->ReloadId();
    const auto& currentScene = sceneInfos_[currentIndex_];
    core::pb::Scene exportScene;
    //auto exportingScene = sceneInfos_[currentIndex_];
    const auto pkgSceneName = currentScene.path + ".pkg";
    //Validate scene
    
    if(currentScene.renderPassId == INVALID_RESOURCE_ID)
    {
        LogWarning("Could not export scene, missing render pass");
        return false;
    }
    std::unordered_map<ResourceId, int> resourceIndexMap;
    std::vector<std::string> cubemapTextures;
    auto* currentRenderPass = renderPassEditor->GetRenderPass(currentScene.renderPassId);
    auto* exportRenderPass = exportScene.mutable_render_pass();
    //*exportRenderPass = currentRenderPass->info;
    for(int subPassIndex = 0; subPassIndex < currentRenderPass->info.sub_passes_size(); subPassIndex++)
    {
        auto* exportSubPass = exportRenderPass->add_sub_passes();
        const auto& editorSubPass = currentRenderPass->info.sub_passes(subPassIndex);

        //export framebuffer
        const auto& framebufferPath = editorSubPass.framebuffer_path();
        if(!framebufferPath.empty())
        {
            const auto framebufferId = resourceManager.FindResourceByPath(framebufferPath);
            if(framebufferId == INVALID_RESOURCE_ID)
            {
                LogWarning(fmt::format("Could not export scene, invalid resource id for framebuffer. Framebuffer path: {}", framebufferPath));
                return false;
            }
            auto* framebufferInfo = framebufferEditor->GetFramebuffer(framebufferId);
            auto framebufferIt = resourceIndexMap.find(framebufferInfo->resourceId);
            if (framebufferIt == resourceIndexMap.end())
            {
                auto index = exportScene.framebuffers_size();
                auto* newFramebuffer = exportScene.add_framebuffers();
                *newFramebuffer = framebufferInfo->info;
                exportSubPass->set_framebuffer_index(index);
                resourceIndexMap[framebufferId] = index;
            }
            else
            {
                exportSubPass->set_framebuffer_index(framebufferIt->second);
            }
        }
        else
        {
            exportSubPass->set_framebuffer_index(-1); // setting back to backbuffer
        }
        //Export commands
        for(int commandIndex = 0; commandIndex < editorSubPass.command_paths_size(); commandIndex++)
        {
            auto* exportCommand = exportSubPass->add_commands();
            const auto editorCommandPath = editorSubPass.command_paths(commandIndex);
            const auto commandId = resourceManager.FindResourceByPath(editorCommandPath);
            if(commandId == INVALID_RESOURCE_ID)
            {
                LogWarning(fmt::format("Could not export scene, missing command in subpass. Command path: {}", editorCommandPath));
                return false;
            }
            const auto* editorCommand = commandEditor->GetCommand(commandId);
            *exportCommand = editorCommand->info.draw_command();
            if (editorCommand->materialId == INVALID_RESOURCE_ID)
            {
                LogWarning(fmt::format("Could not export scene, missing material in command. Command {}", exportCommand->name()));
                return false;
            }
            //link material index
            const auto* material = materialEditor->GetMaterial(editorCommand->materialId);
            auto materialIndexIt = resourceIndexMap.find(material->resourceId);
            if(materialIndexIt == resourceIndexMap.end())
            {
                const auto materialIndex = exportScene.materials_size();
                auto* newMaterial = exportScene.add_materials();
                *newMaterial = material->info.material();
                newMaterial->clear_textures();
                resourceIndexMap[material->resourceId] = materialIndex;

                //list textures
                for(int textureIndex = 0; textureIndex < material->info.textures_size(); textureIndex++)
                {
                    const auto& editorMaterialTexture = material->info.textures(textureIndex);
                    auto* materialTexture = newMaterial->add_textures();
                    if(editorMaterialTexture.texture_name().empty() && editorMaterialTexture.material_texture().attachment_name().empty())
                    {
                        LogWarning(fmt::format(
                            "Could not export, missing texture/attachment in material sampler. Material: {} Sampler: {}", 
                            material->info.material().name(), editorMaterialTexture.material_texture().sampler_name()));
                        materialTexture->set_texture_index(-1);
                        return false;
                    }
                    else if (editorMaterialTexture.material_texture().attachment_name().empty())
                    {
                        auto textureId = resourceManager.FindResourceByPath(editorMaterialTexture.texture_name());
                        if (textureId == INVALID_RESOURCE_ID)
                        {
                            LogWarning(fmt::format("Could not export scene. Missing texture in material sampler, Material: {} Sampler: {}", 
                                material->info.material().name(), editorMaterialTexture.material_texture().sampler_name()));
                            materialTexture->set_texture_index(-1);
                            return false;
                        }
                        auto textureIt = resourceIndexMap.find(textureId);
                        if (textureIt == resourceIndexMap.end())
                        {
                            const auto index = exportScene.textures_size();
                            auto* textureInfo = textureEditor->GetTexture(textureId);
                            *exportScene.add_textures() = textureInfo->info;
                            if (GetFileExtension(textureInfo->info.path()) == ".cube")
                            {
                                for (auto& cubeTexture : textureInfo->cubemap.texture_paths())
                                {
                                    if (!cubeTexture.empty())
                                    {
                                        cubemapTextures.push_back(cubeTexture);
                                    }
                                }
                            }
                            resourceIndexMap[textureId] = index;
                            materialTexture->set_texture_index(index);
                        }
                        else
                        {
                            materialTexture->set_texture_index(textureIt->second);
                        }
                    }
                    else //framebuffer attachment for sampler
                    {
                        bool isValid = false;
                        for(const auto& framebufferPb: currentScene.info.framebuffers())
                        {
                            if(framebufferPb.name() == editorMaterialTexture.material_texture().framebuffer_name())
                            {
                                for(const auto& colorAttachment : framebufferPb.color_attachments())
                                {
                                    if(!colorAttachment.rbo() && colorAttachment.name() == editorMaterialTexture.material_texture().attachment_name())
                                    {
                                        isValid = true;
                                    }
                                }
                                if(framebufferPb.has_depth_stencil_attachment() && !framebufferPb.depth_stencil_attachment().rbo())
                                {
                                    if(framebufferPb.depth_stencil_attachment().name() == editorMaterialTexture.material_texture().attachment_name())
                                    {
                                        isValid = true;
                                    }
                                }
                            }
                        }
                        if(!isValid)
                        {
                            LogWarning(fmt::format("Could not export, invalid framebuffer attachment in material sampler. Material: {} Sampler: {}", 
                                material->info.material().name(), editorMaterialTexture.material_texture().sampler_name()));
                            return false;
                        }
                        materialTexture->set_texture_index(-1);
                    }
                }

                //check if pipeline exists
                if(material->pipelineId == INVALID_RESOURCE_ID)
                {
                    LogWarning(fmt::format("Could not export scene, missing pipeline in material. Material: {}", material->path));
                    return false;
                }
                const auto* pipeline = pipelineEditor->GetPipeline(material->pipelineId);
                auto pipelineIndexIt = resourceIndexMap.find(pipeline->resourceId);
                if(pipelineIndexIt == resourceIndexMap.end())
                {
                    const auto pipelineIndex = exportScene.pipelines_size();
                    auto* newPipeline = exportScene.add_pipelines();
                    *newPipeline = pipeline->info.pipeline();
                    resourceIndexMap[pipeline->resourceId] = pipelineIndex;

                    if(pipeline->vertexShaderId == INVALID_RESOURCE_ID)
                    {
                        LogWarning(fmt::format("Could not export scene, missing vertex shader in pipeline. Pipeline: {}", pipeline->path));
                        return false;
                    }
                    const auto* vertexShader = shaderEditor->GetShader(pipeline->vertexShaderId);
                    auto vertexShaderIt = resourceIndexMap.find(pipeline->vertexShaderId);
                    if(vertexShaderIt == resourceIndexMap.end())
                    {
                        const auto vertexShaderIndex = exportScene.shaders_size();
                        auto* newVertexShader = exportScene.add_shaders();
                        *newVertexShader = vertexShader->info;
                        newVertexShader->set_path(IsVulkanScene()?vertexShader->info.path() + ".spv": vertexShader->info.path());
                        resourceIndexMap[vertexShader->resourceId] = vertexShaderIndex;
                        newPipeline->set_vertex_shader_index(vertexShaderIndex);
                    }
                    else
                    {
                        newPipeline->set_vertex_shader_index(vertexShaderIt->second);
                    }

                    if (pipeline->fragmentShaderId == INVALID_RESOURCE_ID)
                    {
                        LogWarning(fmt::format("Could not export scene, missing fragment shader in pipeline. Pipeline: {}", pipeline->path));
                        return false;
                    }
                    const auto* fragmentShader = shaderEditor->GetShader(pipeline->fragmentShaderId);
                    auto fragmentShaderIt = resourceIndexMap.find(pipeline->fragmentShaderId);
                    if (fragmentShaderIt == resourceIndexMap.end())
                    {
                        const auto fragmentShaderIndex = exportScene.shaders_size();
                        auto* newFragmentShader = exportScene.add_shaders();
                        *newFragmentShader = fragmentShader->info;
                        newFragmentShader->set_path(IsVulkanScene() ? fragmentShader->info.path() + ".spv" : fragmentShader->info.path());
                        resourceIndexMap[fragmentShader->resourceId] = fragmentShaderIndex;
                        newPipeline->set_fragment_shader_index(fragmentShaderIndex);
                    }
                    else
                    {
                        newPipeline->set_fragment_shader_index(fragmentShaderIt->second);
                    }

                    //TODO check comp/geom shaders to get index

                    newMaterial->set_pipeline_index(pipelineIndex);
                }
                else
                {
                    newMaterial->set_pipeline_index(pipelineIndexIt->second);
                }
                exportCommand->set_material_index(materialIndex);
            }
            else
            {
                exportCommand->set_material_index(materialIndexIt->second);
            }
            //link mesh index
            if(editorCommand->meshId == INVALID_RESOURCE_ID)
            {
                LogWarning(fmt::format("Could not export scene, missing mesh in command. Command: {}", editorCommand->path));
                return false;
            }
            auto* mesh = meshEditor->GetMesh(editorCommand->meshId);
            //add model if not done already

            int meshModel = -1;
            if (!mesh->info.model_path().empty())
            {
                for (int modelIndex = 0; modelIndex < currentScene.info.scene().model_paths_size(); modelIndex++)
                {
                    auto& sceneModelPath = currentScene.info.scene().model_paths(modelIndex);
                    if (fs::equivalent(sceneModelPath, mesh->info.model_path()))
                    {
                        meshModel = modelIndex;
                        break;
                    }
                }
                if(meshModel == -1)
                {
                    meshModel = exportScene.model_paths_size();
                    exportScene.add_model_paths(mesh->info.model_path());
                }
                else
                {
                    mesh->info.mutable_mesh()->set_model_index(meshModel);
                }
            }
            auto meshIndexIt = resourceIndexMap.find(mesh->resourceId);
            if(meshIndexIt == resourceIndexMap.end())
            {
                const auto meshIndex = exportScene.meshes_size();
                auto* newMesh = exportScene.add_meshes();
                *newMesh = mesh->info.mesh();
                newMesh->set_model_index(meshModel);
                resourceIndexMap[mesh->resourceId] = meshIndex;
                exportCommand->set_mesh_index(meshIndex);
            }
            else
            {
                exportCommand->set_mesh_index(meshIndexIt->second);
            }
        }
    }
    for(int i = 0; i < currentScene.info.py_system_paths_size();i++)
    {
        const auto& pySystemPath = currentScene.info.py_system_paths(i);
        const auto pySystemId = resourceManager.FindResourceByPath(pySystemPath);
        const auto* pySystemInfo = scriptEditor->GetScriptInfo(pySystemId);

        *exportScene.add_systems() = pySystemInfo->info;
    }
    constexpr std::string_view exportScenePath = "root.scene";
    //Write scene
    std::ofstream fileOut(exportScenePath.data(), std::ios::binary);
    if (!exportScene.SerializeToOstream(&fileOut))
    {
        LogWarning(fmt::format("Could not save scene for export at: {}", exportScenePath));
        return false;
    }
    fileOut.close(); //force write
    //Create scene json
    json sceneJson;
    sceneJson["scene"] = exportScenePath.data();
    std::vector<std::string> shaderPaths;
    shaderPaths.reserve(exportScene.shaders_size());
    for(int i = 0; i < exportScene.shaders_size(); i++)
    {
        shaderPaths.push_back(fs::path(exportScene.shaders(i).path(), std::filesystem::path::generic_format).string());
    }
    sceneJson["shaders"] = shaderPaths;
    std::vector<std::string> scriptPaths;
    scriptPaths.reserve(exportScene.systems_size());
    for (int i = 0; i < exportScene.systems_size(); i++)
    {
        scriptPaths.push_back(fs::path(exportScene.systems(i).path(), std::filesystem::path::generic_format).string());
    }
    sceneJson["scripts"] = scriptPaths;
    std::vector<std::string> texturePaths;
    texturePaths.reserve(exportScene.textures_size());
    for(int i = 0; i < exportScene.textures_size(); i++)
    {
        texturePaths.push_back(exportScene.textures(i).path());
    }
    sceneJson["textures"] = texturePaths;
    sceneJson["cubemap_textures"] = cubemapTextures;
    std::vector<std::string> objPaths;
    objPaths.reserve(exportScene.model_paths_size());
    for(int i = 0; i < exportScene.model_paths_size(); i++)
    {
        objPaths.push_back(exportScene.model_paths(i));
    }
    sceneJson["models"] = objPaths;

    std::vector<std::string> others;
    for(auto& objFile : objPaths)
    {
        const auto modelPath = fmt::format("{}/{}.model", GetFolder(objFile), GetFilename(objFile, false));
        const auto modelId = resourceManager.FindResourceByPath(modelPath);
        auto* model = modelEditor->GetModel(modelId);
        for(int i = 0; i < model->info.mtl_paths_size(); i++)
        {
            others.push_back(model->info.mtl_paths(i));
        }
    }
    sceneJson["others"] = others;

    //Call python function exporting the scene
    try
    {
        py::function exportSceneFunc = py::module_::import("scripts.generate_scene").attr("export_scene");
        exportSceneFunc(pkgSceneName, sceneJson.dump());
    }
    catch (py::error_already_set& e)
    {
        LogError(e.what());
        return false;
    }
    RemoveFile(exportScenePath);
    ExecutePlayer(pkgSceneName);
    return true;
}

void SceneEditor::ReloadId()
{
    auto* editor = Editor::GetInstance();
    const auto& resourceManager = editor->GetResourceManager();
    for(auto& sceneInfo: sceneInfos_)
    {
        if (sceneInfo.renderPassId == INVALID_RESOURCE_ID && !sceneInfo.info.render_pass_path().empty())
        {
            sceneInfo.renderPassId = resourceManager.FindResourceByPath(sceneInfo.info.render_pass_path());
        }
    }
}

void SceneEditor::Delete()
{
    if (currentIndex_ >= sceneInfos_.size())
    {
        return;
    }
    auto* editor = Editor::GetInstance();
    auto& resourceManager = editor->GetResourceManager();
    resourceManager.RemoveResource(sceneInfos_[currentIndex_].path, true);
}

std::span<const std::string_view> SceneEditor::GetExtensions() const
{
    static constexpr std::array<std::string_view, 1> extensions = { ".scene" };
    return std::span{ extensions };
}

void SceneEditor::Clear()
{
    sceneInfos_.clear();
    currentIndex_ = -1;
}

bool SceneEditor::IsVulkanScene() const
{
    if(currentIndex_ == 0)
    {
        return sceneInfos_[0].info.scene().is_vulkan();
    }
    return false;
}

SceneEditor* GetSceneEditor()
{
    const auto* editor = Editor::GetInstance();
    return static_cast<SceneEditor*>(editor->GetEditorSystem(EditorType::SCENE));
}
}
