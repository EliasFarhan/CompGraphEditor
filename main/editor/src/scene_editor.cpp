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
#include "shader_editor.h"

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
}

void SceneEditor::UpdateExistingResource(const Resource& resource)
{
}

bool SceneEditor::CheckExtensions(std::string_view extension)
{
    return extension == ".scene";
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

bool SceneEditor::ExportScene()
{
    if (currentIndex_ >= sceneInfos_.size())
    {
        LogWarning("Could not export no scene selected");
        return false;
    }
    auto* editor = Editor::GetInstance();
    const auto& resourceManager = editor->GetResourceManager();
    const auto* renderPassEditor = dynamic_cast<RenderPassEditor*>(editor->GetEditorSystem(EditorType::RENDER_PASS));
    const auto* commandEditor = dynamic_cast<CommandEditor*>(editor->GetEditorSystem(EditorType::COMMAND));
    const auto* materialEditor = dynamic_cast<MaterialEditor*>(editor->GetEditorSystem(EditorType::MATERIAL));
    const auto* pipelineEditor = dynamic_cast<PipelineEditor*>(editor->GetEditorSystem(EditorType::PIPELINE));
    const auto* shaderEditor = dynamic_cast<ShaderEditor*>(editor->GetEditorSystem(EditorType::SHADER));
    const auto* meshEditor = dynamic_cast<MeshEditor*>(editor->GetEditorSystem(EditorType::MODEL));

    //TODO reload all editors to get all correct resourceId

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
    sceneJson["textures"] = json::array();

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
}
