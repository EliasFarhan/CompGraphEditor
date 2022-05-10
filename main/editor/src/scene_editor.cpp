#include "scene_editor.h"
#include "engine/filesystem.h"
#include "utils/log.h"
#include "render_pass_editor.h"
#include "command_editor.h"

#include <imgui.h>
#include <fmt/format.h>
#include <fstream>

#include "editor.h"


#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <nlohmann/json.hpp>

#include "material_editor.h"
#include "mesh_editor.h"
#include "pipeline_editor.h"
#include "script_editor.h"
#include "shader_editor.h"
#include "texture_editor.h"

namespace py = pybind11;
using json = nlohmann::json;

namespace gpr5300
{
void SceneEditor::AddResource(const Resource& resource)
{
    SceneInfo commandInfo{};
    commandInfo.resourceId = resource.resourceId;
    commandInfo.filename = GetFilename(resource.path);

    const auto extension = GetFileExtension(resource.path);
    if (extension == ".scene")
    {
        const auto& fileSystem = FilesystemLocator::get();
        if (!fileSystem.IsRegularFile(resource.path))
        {
            LogWarning(fmt::format("Could not find scene file: {}", resource.path));
            return;
        }
        std::ifstream fileIn(resource.path, std::ios::binary);
        if (!commandInfo.info.ParseFromIstream(&fileIn))
        {
            LogWarning(fmt::format("Could not open protobuf file: {}", resource.path));
            return;
        }
    }
    commandInfo.path = resource.path;
    sceneInfos_.push_back(commandInfo);
}

void SceneEditor::RemoveResource(const Resource& resource)
{
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

void SceneEditor::DrawMainView()
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

    if(ImGui::CollapsingHeader("Scripts"))
    {
        std::vector<int> removedScripts;
        for (int i = 0; i < currentScene.info.py_system_paths_size(); i++)
        {
            const auto& pySystemPath = currentScene.info.py_system_paths(i);
            std::string name = fmt::format("Script: {}", i);
            const ScriptInfo* pySystem = nullptr;
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
                    pySystem = scriptEditor->GetScriptInfo(pySystemId);
                    name = fmt::format("Script: {}.{}", pySystem->info.module(), pySystem->info.class_());
                }
            }
            bool visible = true;
            if (ImGui::CollapsingHeader(name.c_str(), &visible))
            {
                const auto& pySystems = scriptEditor->GetScriptInfos();
                const auto scriptsId = fmt::format("script {} combo", i);
                ImGui::PushID(scriptsId.c_str());
                if (ImGui::BeginCombo("Available Scripts", pySystem ? pySystem->info.class_().c_str() : "Missing script"))
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


}

bool SceneEditor::DrawContentList(bool unfocus)
{
    bool wasFocused = false;
    if (unfocus)
        currentIndex_ = sceneInfos_.size();
    for (std::size_t i = 0; i < sceneInfos_.size(); i++)
    {
        const auto& sceneInfo = sceneInfos_[i];
        const auto selected = ImGui::Selectable(sceneInfo.filename.data(), currentIndex_ == i);

        if (selected)
        {

            currentIndex_ = i;
            wasFocused = true;

        }
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::Button("Export Scene"))
            {
                currentIndex_ = i;
                wasFocused = true;
                ExportScene();
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
    return wasFocused;
}

std::string_view SceneEditor::GetSubFolder()
{
    return "scenes/";
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

bool SceneEditor::ExportScene() const
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
    const auto* scriptEditor = dynamic_cast<ScriptEditor*>(editor->GetEditorSystem(EditorType::SCRIPT));
    auto* textureEditor = dynamic_cast<TextureEditor*>(editor->GetEditorSystem(EditorType::TEXTURE));
    //TODO reload all editors to get all correct resourceId
    commandEditor->ReloadId();
    const auto& currentScene = sceneInfos_[currentIndex_];
    auto exportScene = sceneInfos_[currentIndex_];
    const auto pkgSceneName = exportScene.path + ".pkg";
    //Validate scene
    exportScene.info.Clear();
    if(currentScene.renderPassId == INVALID_RESOURCE_ID)
    {
        LogWarning("Could not export scene, missing render pass");
        return false;
    }
    std::unordered_map<ResourceId, int> resourceIndexMap;
    auto* currentRenderPass = renderPassEditor->GetRenderPass(currentScene.renderPassId);
    auto* exportRenderPass = exportScene.info.mutable_render_pass();
    *exportRenderPass = currentRenderPass->info;
    for(int subPassIndex = 0; subPassIndex < exportRenderPass->sub_passes_size(); subPassIndex++)
    {
        auto* exportSubPass = exportRenderPass->mutable_sub_passes(subPassIndex);
        
        for(int commandIndex = 0; commandIndex < exportSubPass->command_paths_size(); commandIndex++)
        {
            auto* exportCommand = exportSubPass->add_commands();
            const auto commandPath = exportSubPass->command_paths(commandIndex);
            const auto commandId = resourceManager.FindResourceByPath(commandPath);
            if(commandId == INVALID_RESOURCE_ID)
            {
                LogWarning("Could not export scene, missing command in subpass");
                return false;
            }
            const auto* command = commandEditor->GetCommand(commandId);
            *exportCommand = command->info;
            if (command->materialId == INVALID_RESOURCE_ID)
            {
                LogWarning("Could not export scene, missing material in command");
                return false;
            }
            //link material index
            const auto* material = materialEditor->GetMaterial(command->materialId);
            auto materialIndexIt = resourceIndexMap.find(material->resourceId);
            if(materialIndexIt == resourceIndexMap.end())
            {
                const auto materialIndex = exportScene.info.materials_size();
                auto* newMaterial = exportScene.info.add_materials();
                *newMaterial = material->info;
                resourceIndexMap[material->resourceId] = materialIndex;

                //list textures
                for(int textureIndex = 0; textureIndex < newMaterial->textures_size(); textureIndex++)
                {
                    auto* materialTexture = newMaterial->mutable_textures(textureIndex);
                    auto textureId = resourceManager.FindResourceByPath(materialTexture->texture_name());
                    if(textureId == INVALID_RESOURCE_ID)
                    {
                        LogWarning("Could not export scene, missing texture in material sampler");
                        return false;
                    }
                    auto textureIt = resourceIndexMap.find(textureId);
                    if(textureIt == resourceIndexMap.end())
                    {
                        const auto index = exportScene.info.textures_size();
                        *exportScene.info.add_textures() = textureEditor->GetTexture(textureId)->info;
                        resourceIndexMap[textureId] = index;
                        materialTexture->set_texture_index(index);
                    }
                    else
                    {
                        materialTexture->set_texture_index(textureIt->second);
                    }
                }

                //check if pipeline exists
                if(material->pipelineId == INVALID_RESOURCE_ID)
                {
                    LogWarning("Could not export scene, missing pipeline in material");
                    return false;
                }
                const auto* pipeline = pipelineEditor->GetPipeline(material->pipelineId);
                auto pipelineIndexIt = resourceIndexMap.find(pipeline->resourceId);
                if(pipelineIndexIt == resourceIndexMap.end())
                {
                    const auto pipelineIndex = exportScene.info.pipelines_size();
                    auto* newPipeline = exportScene.info.add_pipelines();
                    *newPipeline = pipeline->info;
                    resourceIndexMap[pipeline->resourceId] = pipelineIndex;

                    if(pipeline->vertexShaderId == INVALID_RESOURCE_ID)
                    {
                        LogWarning("Could not export scene, missing vertex shader in pipeline");
                        return false;
                    }
                    const auto* vertexShader = shaderEditor->GetShader(pipeline->vertexShaderId);
                    auto vertexShaderIt = resourceIndexMap.find(pipeline->vertexShaderId);
                    if(vertexShaderIt == resourceIndexMap.end())
                    {
                        const auto vertexShaderIndex = exportScene.info.shaders_size();
                        auto* newVertexShader = exportScene.info.add_shaders();
                        *newVertexShader = vertexShader->info;
                        resourceIndexMap[vertexShader->resourceId] = vertexShaderIndex;
                        newPipeline->set_vertex_shader_index(vertexShaderIndex);
                    }
                    else
                    {
                        newPipeline->set_vertex_shader_index(vertexShaderIt->second);
                    }

                    if (pipeline->fragmentShaderId == INVALID_RESOURCE_ID)
                    {
                        LogWarning("Could not export scene, missing vertex shader in pipeline");
                        return false;
                    }
                    const auto* fragmentShader = shaderEditor->GetShader(pipeline->fragmentShaderId);
                    auto fragmentShaderIt = resourceIndexMap.find(pipeline->fragmentShaderId);
                    if (fragmentShaderIt == resourceIndexMap.end())
                    {
                        const auto fragmentShaderIndex = exportScene.info.shaders_size();
                        auto* newFragmentShader = exportScene.info.add_shaders();
                        *newFragmentShader = fragmentShader->info;
                        resourceIndexMap[vertexShader->resourceId] = fragmentShaderIndex;
                        newPipeline->set_fragment_shader_index(fragmentShaderIndex);
                    }
                    else
                    {
                        newPipeline->set_fragment_shader_index(vertexShaderIt->second);
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
            if(command->meshId == INVALID_RESOURCE_ID)
            {
                LogWarning("Could not export scene, missing mesh in command");
                return false;
            }
            const auto* mesh = meshEditor->GetMesh(command->meshId);
            auto meshIndexIt = resourceIndexMap.find(mesh->resourceId);
            if(meshIndexIt == resourceIndexMap.end())
            {
                const auto meshIndex = exportScene.info.meshes_size();
                auto* newMesh = exportScene.info.add_meshes();
                *newMesh = mesh->info;
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

        *exportScene.info.add_py_systems() = pySystemInfo->info;
    }
    exportScene.path = "root.scene";
    //Write scene
    std::ofstream fileOut(exportScene.path, std::ios::binary);
    if (!exportScene.info.SerializeToOstream(&fileOut))
    {
        LogWarning(fmt::format("Could not save scene for export at: {}", exportScene.path));
        return false;
    }
    fileOut.close(); //force write
    //Create scene json
    json sceneJson;
    sceneJson["scene"] = exportScene.path;
    std::vector<std::string> shaderPaths;
    shaderPaths.reserve(currentScene.info.shaders_size());
    for(int i = 0; i < exportScene.info.shaders_size(); i++)
    {
        shaderPaths.push_back(fs::path(exportScene.info.shaders(i).path(), std::filesystem::path::generic_format).string());
    }
    sceneJson["shaders"] = shaderPaths;
    std::vector<std::string> scriptPaths;
    scriptPaths.reserve(exportScene.info.py_systems_size());
    for (int i = 0; i < exportScene.info.py_systems_size(); i++)
    {
        scriptPaths.push_back(fs::path(exportScene.info.py_systems(i).path(), std::filesystem::path::generic_format).string());
    }
    sceneJson["scripts"] = scriptPaths;
    std::vector<std::string> texturePaths;
    texturePaths.reserve(exportScene.info.textures_size());
    for(int i = 0; i < exportScene.info.textures_size(); i++)
    {
        texturePaths.push_back(exportScene.info.textures(i).path());
    }
    sceneJson["textures"] = texturePaths;

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
    RemoveFile(exportScene.path);
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
    resourceManager.RemoveResource(sceneInfos_[currentIndex_].path);
}

std::span<const std::string_view> SceneEditor::GetExtensions() const
{
    static constexpr std::array<std::string_view, 1> extensions = { ".scene" };
    return std::span{ extensions };
}
}
