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
#include "pipeline_editor.h"

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

    if(currentScene.renderPassId == INVALID_RESOURCE_ID && !currentScene.info.render_pass_path().empty())
    {
        currentScene.renderPassId = resourceManager.FindResourceByPath(currentScene.info.render_pass_path());
    }

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
    auto* renderPassEditor = dynamic_cast<RenderPassEditor*>(editor->GetEditorSystem(EditorType::RENDER_PASS));
    auto* commandEditor = dynamic_cast<CommandEditor*>(editor->GetEditorSystem(EditorType::COMMAND));
    auto* materialEditor = dynamic_cast<MaterialEditor*>(editor->GetEditorSystem(EditorType::MATERIAL));
    auto* pipelineEditor = dynamic_cast<PipelineEditor*>(editor->GetEditorSystem(EditorType::PIPELINE));
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

                    //TODO check all shaders vert/frag/comp/geom to get index


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
            //TODO link mesh index
        }
    }

    //Create scene json
    json sceneJson;

    //Call python function exporting the scene
    py::function exportSceneFunc = py::module_::import("scripts.generate_scene").attr("export_scene");


    return true;
}
}
