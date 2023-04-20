#include "model_editor.h"
#include <array>
#include "utils/log.h"
#include <fmt/format.h>

#include "editor.h"
#include "mesh_editor.h"
#include "resource_manager.h"
#include "texture_editor.h"
#include "engine/filesystem.h"

#include <fstream>

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <nlohmann/json.hpp>

#include "command_editor.h"
#include "material_editor.h"
#include "pipeline_editor.h"
#include "scene_editor.h"

using json = nlohmann::json;
namespace py = pybind11;

namespace editor
{
void ModelEditor::AddResource(const Resource& resource)
{
    const auto extension = GetFileExtension(resource.path);
    if(extension != ".model")
        return;
    ModelInfo modelInfo{};
    modelInfo.resourceId = resource.resourceId;
    modelInfo.filename = GetFilename(resource.path);

    const auto& fileSystem = core::FilesystemLocator::get();
    if (!fileSystem.IsRegularFile(resource.path))
    {
        LogWarning(fmt::format("Could not find model file: {}", resource.path));
        return;
    }
    std::ifstream fileIn(resource.path, std::ios::binary);
    if (!modelInfo.info.ParseFromIstream(&fileIn))
    {
        LogWarning(fmt::format("Could not open protobuf file: {}", resource.path));
        return;
    }
    auto& modelManager = core::GetModelManager();
    const auto& modelPath = modelInfo.info.model_path();

    modelInfo.modelIndex = modelManager.ImportModel(modelPath);
    if (modelInfo.modelIndex == core::INVALID_MODEL_INDEX)
    {
        LogError(fmt::format("Error parsing obj file: {}", modelPath));
        return;

    }
    modelInfo.path = resource.path;
    modelInfos_.push_back(modelInfo);
}

void ModelEditor::RemoveResource(const Resource& resource)
{
}

void ModelEditor::UpdateExistingResource(const Resource& resource)
{
}



void ModelEditor::DrawInspector()
{
    if(currentIndex_ >= modelInfos_.size())
    {
        return;
    }
    auto& filesystem = core::FilesystemLocator::get();
    auto& currentModelInfo = modelInfos_[currentIndex_];
    const auto baseDir = GetFolder(currentModelInfo.path);
    auto* editor = Editor::GetInstance();
    auto& resourceManager = editor->GetResourceManager();
    const auto* pipelineEditor = dynamic_cast<PipelineEditor*>(editor->GetEditorSystem(EditorType::PIPELINE));
    auto* materialEditor = dynamic_cast<MaterialEditor*>(editor->GetEditorSystem(EditorType::MATERIAL));
    auto* commandEditor = dynamic_cast<CommandEditor*>(editor->GetEditorSystem(EditorType::COMMAND));
    ImGui::Text("Path: %s", currentModelInfo.info.model_path().c_str());

    if (ImGui::BeginListBox("Meshes"))
    {
        for (int i = 0; i < currentModelInfo.info.meshes_size(); i++)
        {
            const auto& meshInfo = currentModelInfo.info.meshes(i);
            const auto& text = meshInfo.mesh_name();
            ImGui::Selectable(text.c_str(), false);
        }
        ImGui::EndListBox();
    }

    if (ImGui::BeginListBox("Materials"))
    {
        for (int i = 0; i < currentModelInfo.info.materials_size(); i++)
        {
            const auto& materialInfo = currentModelInfo.info.materials(i);
            const auto& text = materialInfo.material_name();
            ImGui::Selectable(text.c_str(), false);
        }
        ImGui::EndListBox();
    }

    if (ImGui::BeginListBox("Textures"))
    {
        for (int i = 0; i < currentModelInfo.info.textures_size(); i++)
        {
            const auto& textureInfo = currentModelInfo.info.textures(i);
            const auto& texturePath = textureInfo.texture_path();
            ImGui::Selectable(texturePath.c_str(), false);
            ImGui::SameLine();

            ImGui::Text("%s", aiTextureTypeToString(static_cast<aiTextureType>(textureInfo.type())));
        }
        ImGui::EndListBox();
    }

    if(currentModelInfo.drawCommands.size() != currentModelInfo.info.draw_commands_size())
    {
        ReloadDrawCommands(currentIndex_);
    }

    ImGui::Separator();
    for(int commandIndex = 0; commandIndex < currentModelInfo.info.draw_commands_size(); commandIndex++)
    {
        auto* drawCommandInfo = currentModelInfo.info.mutable_draw_commands(commandIndex);
        auto& drawCommand = currentModelInfo.drawCommands[commandIndex];
        auto headerName = fmt::format("Draw Command {}", commandIndex);
        if (ImGui::CollapsingHeader(headerName.c_str()))
        {
            auto pipelineId = fmt::format("{} Set Pipeline", headerName);
            ImGui::PushID(pipelineId.c_str());
            const auto& pipelines = pipelineEditor->GetPipelines();
            auto pipelinePreview = drawCommandInfo->pipeline_path().empty() ? "" : GetFilename(drawCommandInfo->pipeline_path());
            if (ImGui::BeginCombo("Pipeline", pipelinePreview.empty() ? "Empty pipeline" : pipelinePreview.c_str()))
            {
                for (const auto& pipeline : pipelines)
                {
                    if (ImGui::Selectable(pipeline.filename.c_str(), pipeline.resourceId == drawCommand.pipelineId))
                    {
                        drawCommand.pipelineId = pipeline.resourceId;
                        drawCommandInfo->set_pipeline_path(pipeline.path);
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopID();
            if (drawCommand.pipelineId != INVALID_RESOURCE_ID)
            {
                auto buttonId = fmt::format("{} Generate Button", headerName);
                ImGui::PushID(buttonId.c_str());
                if (ImGui::Button("Generate Materials & Commands"))
                {
                    GenerateMaterialsAndCommands(commandIndex);
                }
                ImGui::PopID();
            }
        }
        ImGui::Separator();
    }
    if(ImGui::Button("Add New Command"))
    {
        currentModelInfo.info.add_draw_commands();
    }
}

bool ModelEditor::DrawContentList(bool unfocus)
{
    bool wasFocused = false;
    if (unfocus)
        currentIndex_ = modelInfos_.size();
    for (std::size_t i = 0; i < modelInfos_.size(); i++)
    {
        const auto& meshInfo = modelInfos_[i];
        if (ImGui::Selectable(meshInfo.filename.data(), currentIndex_ == i))
        {
            currentIndex_ = i;
            wasFocused = true;
        }
    }
    return wasFocused;
}

std::string_view ModelEditor::GetSubFolder()
{
    return "models/";
}

EditorType ModelEditor::GetEditorType()
{
    return EditorType::MODEL;
}

void ModelEditor::Save()
{
    for (auto& modelInfo : modelInfos_)
    {
        std::ofstream fileOut(modelInfo.path, std::ios::binary);
        if (!modelInfo.info.SerializeToOstream(&fileOut))
        {
            LogWarning(fmt::format("Could not save model at: {}", modelInfo.path));
        }

    }
}

void ModelEditor::ReloadId()
{
    for(std::size_t i = 0; i < modelInfos_.size(); i++)
    {
        ReloadDrawCommands(i);
    }
}

void ModelEditor::Delete()
{
}

std::span<const std::string_view> ModelEditor::GetExtensions() const
{
    static constexpr std::array<std::string_view, 2> extensions = { ".obj", ".model"};
    return std::span{ extensions };
}

ModelInfo* ModelEditor::GetModel(ResourceId resourceId)
{
    auto it = std::ranges::find_if(modelInfos_, [&resourceId](const auto& modelInfo)
        {
            return modelInfo.resourceId == resourceId;
        });
    if(it != modelInfos_.end())
    {
        return &*it;
    }
    return nullptr;
}

void ModelEditor::ImportResource(std::string_view path)
{
    auto& filesystem = core::FilesystemLocator::get();
    if(GetFileExtension(path) != ".obj")
    {
        LogError("Can only import obj file");
        return;
    }

    auto& modelManager = core::GetModelManager();
    const auto modelId = modelManager.ImportModel(path.data());

    if (modelId == core::INVALID_MODEL_INDEX)
    {
        LogError(fmt::format("Error parsing obj file: {}", path));
        return;

    }
    std::vector<std::string> mtlFiles;
    try
    {
        py::function getMaterialsFromObjFunc = py::module_::import("scripts.obj_parser").attr("get_materials_path");
        std::string result = static_cast<py::str>(getMaterialsFromObjFunc(path));
        auto materialsJson = json::parse(result);
        auto mtlList = materialsJson["materials"];
        for(auto& mtlPath: mtlList)
        {
            mtlFiles.push_back(mtlPath);
        }
    }
    catch(py::error_already_set& e)
    {
        LogError("Could not parse obj to get mtl files");
    }
    catch(json::parse_error& e)
    {
        LogError("Could not parse oobj info from script");
    }
    const auto srcFolder = GetFolder(path);

    const auto& sceneName = GetSceneEditor()->GetCurrentSceneInfo()->info.name();
    const auto dstFolder = fmt::format("{}{}/{}{}/", ResourceManager::dataFolder, sceneName, GetSubFolder(), GetFilename(path, false));
    auto* editor = Editor::GetInstance();
    auto& resourceManager = editor->GetResourceManager();
    CreateNewDirectory(dstFolder);

    editor::pb::EditorModel newModel;
    
    auto findTextureInModelFunc = [&newModel](const std::string_view texture)
    {
        for(int i = 0; i < newModel.textures_size(); i++)
        {
            if(fs::equivalent(newModel.textures(i).texture_path(),texture))
            {
                return i;
            }
        }
        return -1;
    };
    const auto& model = modelManager.GetModel(modelId);

    const auto materials = model.GetMaterials();
    
    auto loadMaterialTextureFunc = [&]
    (std::string_view textureName, core::pb::TextureType textureType, editor::pb::ModelMaterial* material)
    {
        if(textureName.empty())
        {
            return;
        }
        const auto textureSrcPath = fmt::format("{}/{}", srcFolder, textureName);
        const auto textureDstPath = fmt::format("{}{}", dstFolder, textureName);

        int index = -1;
        if(filesystem.FileExists(textureDstPath))
        {
            index = findTextureInModelFunc(textureDstPath);
        }
        else
        {
            if (!CopyFileFromTo(textureSrcPath, textureDstPath))
            {
                LogError(fmt::format("Could not copy texture from {} to {}", textureSrcPath, textureDstPath));
                return;
            }
            resourceManager.AddResource(textureDstPath);
            index = newModel.textures_size();
            auto* newTexture = newModel.add_textures();
            newTexture->set_type(textureType);
            newTexture->set_texture_path(textureDstPath);
            //change the actual texture info to the type to be linked automatically when binding pipeline
            const auto textureId = resourceManager.FindResourceByPath(textureDstPath);
            auto* textureEditor = dynamic_cast<TextureEditor*>(editor->GetEditorSystem(EditorType::TEXTURE));
            auto* texture = textureEditor->GetTexture(textureId);
            texture->info.set_type(textureType);
        }
        material->add_texture_indices(index);
    };

    for (auto& material : materials)
    {
        
        auto* newMaterial = newModel.add_materials();
        newMaterial->set_material_name(material.name);

        for(int i = 0; i < static_cast<int>(core::pb::TextureType::LENGTH); i++)
        {
            loadMaterialTextureFunc(material.textures[i], static_cast<core::pb::TextureType>(i), newMaterial);
        }
    }
    auto modelDstPath = fmt::format("{}{}", dstFolder, GetFilename(path));

    auto meshes = model.GetMeshes();
    for (auto& shape : meshes)
    {
        
        auto meshInfoDstPath = fmt::format("{}{}.mesh", dstFolder, shape.name);
        auto meshInfoId = resourceManager.FindResourceByPath(meshInfoDstPath);
        if (meshInfoId == INVALID_RESOURCE_ID)
        {
            editor->CreateNewFile(meshInfoDstPath, EditorType::MESH);
        }
        meshInfoId = resourceManager.FindResourceByPath(meshInfoDstPath);
        auto* meshEditor = dynamic_cast<MeshEditor*>(editor->GetEditorSystem(EditorType::MESH));
        auto* meshInfo = meshEditor->GetMesh(meshInfoId);
        meshInfo->info.mutable_mesh()->set_primitve_type(core::pb::Mesh_PrimitveType_MODEL);
        meshInfo->info.set_model_path(modelDstPath);
        meshInfo->info.mutable_mesh()->set_mesh_name(shape.name);
        auto* newMesh = newModel.add_meshes();
        newMesh->set_mesh_name(shape.name);
        newMesh->set_material_name(newModel.materials(shape.materialIndex).material_name());
        newMesh->set_mesh_path(meshInfoDstPath);
    }


    for(std::string_view mtlPath: mtlFiles)
    {
        auto mtlSrcPath = fmt::format("{}/{}", srcFolder, GetFilename(mtlPath));
        auto mtlDstPath = fmt::format("{}{}", dstFolder, GetFilename(mtlPath));
        CopyFileFromTo(mtlSrcPath, mtlDstPath);
        *newModel.add_mtl_paths() = mtlDstPath;
    }

    CopyFileFromTo(path, modelDstPath);
    resourceManager.AddResource(modelDstPath);
    newModel.set_model_path(modelDstPath);

    auto modelInfoPath = fmt::format("{}{}.model", dstFolder, GetFilename(path, false));
    filesystem.WriteString(modelInfoPath, newModel.SerializeAsString());
    resourceManager.AddResource(modelInfoPath);

}

void ModelEditor::Clear()
{
    modelInfos_.clear();
    currentIndex_ = -1;
}

void ModelEditor::GenerateMaterialsAndCommands(int commandIndex)
{
    auto& currentModelInfo = modelInfos_[currentIndex_];
    auto* drawCommandInfo = currentModelInfo.info.mutable_draw_commands(commandIndex);
    auto& drawCommand = currentModelInfo.drawCommands[commandIndex];
    
    const auto baseDir = GetFolder(currentModelInfo.path);
    auto* editor = Editor::GetInstance();
    auto& resourceManager = editor->GetResourceManager();
    const auto* pipelineEditor = dynamic_cast<PipelineEditor*>(editor->GetEditorSystem(EditorType::PIPELINE));
    auto* materialEditor = dynamic_cast<MaterialEditor*>(editor->GetEditorSystem(EditorType::MATERIAL));
    auto* commandEditor = dynamic_cast<CommandEditor*>(editor->GetEditorSystem(EditorType::COMMAND));
    const auto* pipeline = pipelineEditor->GetPipeline(drawCommand.pipelineId);
    auto pipelineName = GetFilename(pipeline->path, false);
    drawCommandInfo->mutable_material_paths()->Reserve(currentModelInfo.info.materials_size());
    drawCommand.materialIds.clear();
    drawCommand.drawCommandIds.clear();
    for (int modelMaterialIndex = 0; modelMaterialIndex < currentModelInfo.info.materials_size(); modelMaterialIndex++)
    {
        auto& modelMaterial = currentModelInfo.info.materials(modelMaterialIndex);
        if (drawCommandInfo->material_paths_size() >= modelMaterialIndex)
        {
            drawCommandInfo->add_material_paths();
        }
        //create new material, or reset old one
        auto materialId = resourceManager.FindResourceByPath(drawCommandInfo->material_paths(modelMaterialIndex));
        if (materialId == INVALID_RESOURCE_ID)
        {
            auto materialPath = fmt::format("{}/{}_{}.mat", baseDir, currentModelInfo.info.materials(modelMaterialIndex).material_name(), pipelineName);
            editor->CreateNewFile(materialPath, EditorType::MATERIAL);
            drawCommandInfo->set_material_paths(modelMaterialIndex, materialPath);
            materialId = resourceManager.FindResourceByPath(materialPath);
        }
        drawCommand.materialIds.push_back(materialId);
        auto* material = materialEditor->GetMaterial(materialId);
        material->info.set_pipeline_path(pipeline->path);
        material->pipelineId = pipeline->resourceId;
        material->info.mutable_material()->set_name(modelMaterial.material_name());
        materialEditor->UpdateExistingResource(*resourceManager.GetResource(pipeline->resourceId));

        //Linking textures from model material
        for (int materialTextureIndex = 0; materialTextureIndex < material->info.textures_size(); materialTextureIndex++)
        {
            auto* materialTexture = material->info.mutable_textures(materialTextureIndex);
            if (materialTexture->texture_type() == core::pb::NONE)
                continue;
            for (int modelMaterialTextureIndex = 0; modelMaterialTextureIndex < modelMaterial.texture_indices_size(); modelMaterialTextureIndex++)
            {
                auto& modelTexture = currentModelInfo.info.textures(modelMaterial.texture_indices(modelMaterialTextureIndex));
                if (modelTexture.type() != core::pb::NONE && modelTexture.type() == materialTexture->texture_type())
                {
                    materialTexture->set_texture_name(modelTexture.texture_path());
                    break;
                }
            }
            
        }
    }
    drawCommandInfo->mutable_draw_command_paths()->Reserve(currentModelInfo.info.meshes_size());
    for (int meshIndex = 0; meshIndex < currentModelInfo.info.meshes_size(); meshIndex++)
    {
        auto& modelMesh = currentModelInfo.info.meshes(meshIndex);

        if (drawCommandInfo->draw_command_paths_size() <= meshIndex)
        {
            drawCommandInfo->add_draw_command_paths();
        }
        //create new command, or reset old one
        auto commandId = resourceManager.FindResourceByPath(drawCommandInfo->draw_command_paths(meshIndex));
        if (commandId == INVALID_RESOURCE_ID)
        {
            auto commandPath = fmt::format("{}/{}_{}.cmd", baseDir, currentModelInfo.info.meshes(meshIndex).mesh_name(), pipelineName);
            editor->CreateNewFile(commandPath, EditorType::COMMAND);
            drawCommandInfo->set_draw_command_paths(meshIndex, commandPath);
            commandId = resourceManager.FindResourceByPath(commandPath);
        }
        drawCommand.drawCommandIds.push_back(commandId);
        auto* command = commandEditor->GetCommand(commandId);

        auto modelMaterialName = modelMesh.material_name();
        auto materialPath = fmt::format("{}/{}_{}.mat", baseDir, modelMaterialName, pipelineName);
        auto materialId = resourceManager.FindResourceByPath(materialPath);
        auto meshId = resourceManager.FindResourceByPath(modelMesh.mesh_path());

        command->info.set_mesh_path(modelMesh.mesh_path());
        command->info.set_material_path(materialPath);
        command->materialId = materialId;
        command->meshId = meshId;
        command->info.mutable_draw_command()->set_draw_elements(true);
        const auto& modelManager = core::GetModelManager();
        const auto& model = modelManager.GetModel(currentModelInfo.modelIndex);
        for (auto& mesh : model.GetMeshes())
        {
            if (mesh.name == modelMesh.mesh_name())
            {
                command->info.mutable_draw_command()->set_count(mesh.indices.size());
                break;
            }
        }
    }
}

void ModelEditor::ReloadDrawCommands(std::size_t modelIndex)
{
    auto* editor = Editor::GetInstance();
    const auto& resourceManager = editor->GetResourceManager();
    auto& modelInfo = modelInfos_[modelIndex];
    if(modelInfo.drawCommands.size() != modelInfo.info.draw_commands_size())
    {
        modelInfo.drawCommands.resize(modelInfo.info.draw_commands_size());
    }
    for(int i = 0; i < modelInfo.info.draw_commands_size(); i++)
    {
        auto& drawCommand = modelInfo.drawCommands[i];
        auto& drawCommandInfo = modelInfo.info.draw_commands(i);
        if(drawCommand.materialIds.size() != modelInfo.info.materials_size())
        {
            drawCommand.materialIds.resize(modelInfo.info.materials_size(), INVALID_RESOURCE_ID);
        }
        if(drawCommand.drawCommandIds.size() != modelInfo.info.meshes_size())
        {
            drawCommand.drawCommandIds.resize(modelInfo.info.meshes_size());
        }

        for(int j = 0; j < drawCommandInfo.material_paths_size(); j++)
        {
            if(drawCommand.materialIds[j] == INVALID_RESOURCE_ID)
            {
                drawCommand.materialIds[j] = resourceManager.FindResourceByPath(drawCommandInfo.material_paths(j));
            }
        }
        for(int j = 0; j < drawCommandInfo.draw_command_paths_size(); j++)
        {
            if(drawCommand.drawCommandIds[j] == INVALID_RESOURCE_ID)
            {
                drawCommand.drawCommandIds[j] = resourceManager.FindResourceByPath(drawCommandInfo.draw_command_paths(j));
            }
        }
        if(drawCommand.pipelineId == INVALID_RESOURCE_ID)
        {
            drawCommand.pipelineId = resourceManager.FindResourceByPath(drawCommandInfo.pipeline_path());
        }
    }
}
}
