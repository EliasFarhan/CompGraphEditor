#include "command_editor.h"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <fmt/format.h>

#include "editor.h"
#include "buffer_editor.h"
#include "material_editor.h"
#include "mesh_editor.h"
#include "render_pass_editor.h"
#include "engine/filesystem.h"
#include "utils/log.h"

#include <array>
#include <fstream>

#include "shader_editor.h"

namespace editor
{
void CommandEditor::AddResource(const Resource& resource)
{
    CommandInfo commandInfo{};
    commandInfo.resourceId = resource.resourceId;
    commandInfo.filename = GetFilename(resource.path);

    const auto extension = GetFileExtension(resource.path);
    if (extension == ".cmd")
    {
        const auto& fileSystem = core::FilesystemLocator::get();
        if (!fileSystem.IsRegularFile(resource.path.c_str()))
        {
            LogWarning(fmt::format("Could not find command file: {}", resource.path));
            return;
        }
        std::ifstream fileIn(resource.path.c_str(), std::ios::binary);
        commandInfo.info.emplace<pb::EditorDrawCommand>();
        auto& info = std::get<pb::EditorDrawCommand>(commandInfo.info);
        if (!info.ParseFromIstream(&fileIn))
        {
            LogWarning(fmt::format("Could not open protobuf file: {}", resource.path));
            return;
        }
        if (info.draw_command().name().empty())
        {
            info.mutable_draw_command()->set_name(GetFilename(resource.path, false));
        }
    }
    else if(extension ==".compcmd")
    {
        const auto& fileSystem = core::FilesystemLocator::get();
        if (!fileSystem.IsRegularFile(resource.path.c_str()))
        {
            LogWarning(fmt::format("Could not find command file: {}", resource.path));
            return;
        }
        std::ifstream fileIn(resource.path.c_str(), std::ios::binary);
        commandInfo.info.emplace<pb::EditorComputeCommand>();
        auto& info = std::get<pb::EditorComputeCommand>(commandInfo.info);
        if (!info.ParseFromIstream(&fileIn))
        {
            LogWarning(fmt::format("Could not open protobuf file: {}", resource.path));
            return;
        }
        if (info.compute_command().name().empty())
        {
            info.mutable_compute_command()->set_name(GetFilename(resource.path, false));
        }
    }
    
    commandInfo.path = resource.path;
    commandInfos_.push_back(commandInfo);
}

void CommandEditor::RemoveResource(const Resource& resource)
{
    for(auto& command : commandInfos_)
    {
        if(command.materialId == resource.resourceId)
        {
            if (command.info.index() == 0)
            {
                std::get<pb::EditorDrawCommand>(command.info).clear_material_path();
            }
            else
            {
                std::get<pb::EditorComputeCommand>(command.info).clear_material_path();
            }
            command.materialId = INVALID_RESOURCE_ID;
        }

        if(command.meshId == resource.resourceId)
        {
            if (command.info.index() == 0)
            {
                std::get<pb::EditorDrawCommand>(command.info).clear_mesh_path();
            }
            command.meshId = INVALID_RESOURCE_ID;
        }
    }

    const auto it = std::ranges::find_if(commandInfos_, [&resource](const auto& command)
        {
            return resource.resourceId == command.resourceId;
        });
    if(it != commandInfos_.end())
    {
        commandInfos_.erase(it);
        const auto* editor = Editor::GetInstance();
        auto* renderPassEditor = dynamic_cast<RenderPassEditor*>(editor->GetEditorSystem(EditorType::RENDER_PASS));
        renderPassEditor->RemoveResource(resource);
    }
}

void CommandEditor::UpdateExistingResource(const Resource& resource)
{
}


void CommandEditor::DrawInspector()
{
    if (currentIndex_ >= commandInfos_.size())
    {
        return;
    }
    const auto* editor = Editor::GetInstance();
    auto* materialEditor = dynamic_cast<MaterialEditor*>(editor->GetEditorSystem(EditorType::MATERIAL));
    auto* meshEditor = dynamic_cast<MeshEditor*>(editor->GetEditorSystem(EditorType::MESH));
    auto* pipelineEditor = dynamic_cast<PipelineEditor*>(editor->GetEditorSystem(EditorType::PIPELINE));
    auto* shaderEditor = dynamic_cast<ShaderEditor*>(editor->GetEditorSystem(EditorType::SHADER));
    auto* bufferEditor = dynamic_cast<BufferEditor*>(editor->GetEditorSystem(EditorType::BUFFER));
    auto& currentCommand = commandInfos_[currentIndex_];

    //name editor
    if (currentCommand.info.index() == 0)
    {
        auto& drawCommandInfo = std::get<pb::EditorDrawCommand>(currentCommand.info);
        std::string drawCommandName = drawCommandInfo.draw_command().name();
        if (ImGui::InputText("Name: ", &drawCommandName))
        {
            drawCommandInfo.mutable_draw_command()->set_name(drawCommandName);
        }

        const auto& materials = materialEditor->GetMaterials();
        const auto* materialInfo = materialEditor->GetMaterial(currentCommand.materialId);
        if (ImGui::BeginCombo("Material", materialInfo ? materialInfo->filename.c_str() : "Empty Material"))
        {
            for (auto& material : materials)
            {
                if (ImGui::Selectable(material.filename.c_str(), material.resourceId == currentCommand.materialId))
                {
                    currentCommand.materialId = material.resourceId;
                    drawCommandInfo.set_material_path(material.path.c_str());
                }
            }
            ImGui::EndCombo();
        }
        const auto& meshes = meshEditor->GetMeshes();
        const auto* meshInfo = meshEditor->GetMesh(currentCommand.meshId);
        if (ImGui::BeginCombo("Mesh", meshInfo ? meshInfo->filename.c_str() : "Empty Mesh"))
        {
            for (auto& mesh : meshes)
            {
                if (ImGui::Selectable(mesh.filename.c_str(), mesh.resourceId == currentCommand.meshId))
                {
                    currentCommand.meshId = mesh.resourceId;
                    drawCommandInfo.set_mesh_path(mesh.path.c_str());
                }
            }
            ImGui::EndCombo();
        }
        if (materialInfo == nullptr)
            return;
        auto* pipelineInfo = pipelineEditor->GetPipeline(materialInfo->pipelineId);
        if (pipelineInfo == nullptr)
            return;

        std::array resourceIds = {
            pipelineInfo->vertexShaderId,
            pipelineInfo->fragmentShaderId,
            pipelineInfo->geometryShaderId,
            pipelineInfo->tessControlShaderId,
            pipelineInfo->tessEvalShaderId,
            pipelineInfo->computeShaderId,
            pipelineInfo->rayGenShaderId,
            pipelineInfo->anyHitShaderId,
            pipelineInfo->closestHitShaderId,
            pipelineInfo->intersectionHitShaderId,
            pipelineInfo->missHitShaderId,
        };
        core::pb::Attribute ssbo{};
        ssbo.set_binding(-1);
        const auto checkSsboPresence = [&resourceIds, shaderEditor, pipelineInfo, &ssbo](ResourceId shaderId)
        {
            if (shaderId == INVALID_RESOURCE_ID)
                return;

            const auto* shaderInfo = shaderEditor->GetShader(pipelineInfo->vertexShaderId);
            if (shaderInfo->info.storage_buffers_size() == 0)
                return;
            const auto& storage_buffer = shaderInfo->info.storage_buffers(0);
            ssbo.set_binding(storage_buffer.binding());
            ssbo.set_name(storage_buffer.name());
        };
        for (const auto resourceId : resourceIds)
        {
            checkSsboPresence(resourceId);
        }
        if (ssbo.binding() != -1)
        {
            const auto buffers = bufferEditor->GetBuffers();
            const core::Path currentBufferPath{drawCommandInfo.buffer_path()};

            if (ImGui::BeginCombo("Storage Buffer", currentBufferPath.empty() ? "No Buffer" : currentBufferPath.c_str()))
            {
                for (const auto& buffer : buffers)
                {
                    if (ImGui::Selectable(buffer.path.c_str(), !currentBufferPath.empty() && buffer.path == currentBufferPath))
                    {
                        drawCommandInfo.set_buffer_path(buffer.path.c_str());
                        currentCommand.bufferId = buffer.resourceId;
                    }
                }
                ImGui::EndCombo();
            }
        }

        switch (pipelineInfo->info.pipeline().type())
        {
        case core::pb::Pipeline_Type_RASTERIZE:
        {
            bool automaticDraw = drawCommandInfo.draw_command().automatic_draw();
            if (ImGui::Checkbox("Automatic Draw", &automaticDraw))
            {
                drawCommandInfo.mutable_draw_command()->set_automatic_draw(automaticDraw);
            }

            //Model matrix
            core::pb::Transform* transform = nullptr;
            if (drawCommandInfo.draw_command().has_model_transform())
            {
                transform = drawCommandInfo.mutable_draw_command()->mutable_model_transform();
            }

            if (transform == nullptr)
            {
                if (ImGui::Button("Add Model Transform Matrix"))
                {
                    transform = drawCommandInfo.mutable_draw_command()->mutable_model_transform();
                    auto* position = transform->mutable_position();
                    auto* scale = transform->mutable_scale();
                    scale->set_x(1.0f);
                    scale->set_y(1.0f);
                    scale->set_z(1.0f);
                    auto* eulerAngles = transform->mutable_euler_angles();
                }
            }
            else
            {
                auto* position = transform->mutable_position();
                std::array<float, 3> positionTmp = { {position->x(), position->y(), position->z()} };
                if (ImGui::InputFloat3("Position", positionTmp.data()))
                {
                    position->set_x(positionTmp[0]);
                    position->set_y(positionTmp[1]);
                    position->set_z(positionTmp[2]);
                }
                auto* scale = transform->mutable_scale();
                std::array<float, 3> scaleTmp = { {scale->x(), scale->y(), scale->z()} };
                if (ImGui::InputFloat3("Scale", scaleTmp.data()))
                {
                    scale->set_x(scaleTmp[0]);
                    scale->set_y(scaleTmp[1]);
                    scale->set_z(scaleTmp[2]);
                }
                auto* eulerAngles = transform->mutable_euler_angles();
                std::array<float, 3> eulerAnglesTmp = { {eulerAngles->x(), eulerAngles->y(), eulerAngles->z()} };
                if (ImGui::InputFloat3("Euler Angles", eulerAnglesTmp.data()))
                {
                    eulerAngles->set_x(eulerAnglesTmp[0]);
                    eulerAngles->set_y(eulerAnglesTmp[1]);
                    eulerAngles->set_z(eulerAnglesTmp[2]);
                }
                if (ImGui::Button("Remove Model Transform Matrix"))
                {
                    drawCommandInfo.mutable_draw_command()->clear_model_transform();
                }
            }

            UpdateMeshInCommand(currentIndex_);
            if (meshInfo != nullptr)
            {
                switch (meshInfo->info.mesh().primitve_type())
                {
                case core::pb::Mesh_PrimitveType_NONE:
                {
                    int count = drawCommandInfo.mutable_draw_command()->count();
                    if (ImGui::InputInt("Vertex Count", &count))
                    {
                        drawCommandInfo.mutable_draw_command()->set_count(count);
                    }
                    bool drawElements = drawCommandInfo.mutable_draw_command()->draw_elements();
                    if (ImGui::Checkbox("Draw Elements", &drawElements))
                    {
                        drawCommandInfo.mutable_draw_command()->set_draw_elements(drawElements);
                    }

                    break;
                }
                case core::pb::Mesh_PrimitveType_MODEL:
                {
                    ImGui::Text("Vertex Count: %d", drawCommandInfo.draw_command().count());
                    break;
                }
                default:
                    break;
                }
            }
        }
        case core::pb::Pipeline_Type_COMPUTE:
        {
            break;
        }
        default:
            break;
        }
    }
    else
    {
        //Compute shader
        auto& computeCommandInfo = std::get<pb::EditorComputeCommand>(currentCommand.info);
        std::string drawCommandName = computeCommandInfo.compute_command().name();
        if (ImGui::InputText("Name: ", &drawCommandName))
        {
            computeCommandInfo.mutable_compute_command()->set_name(drawCommandName);
        }

        const auto& materials = materialEditor->GetMaterials();
        const auto* materialInfo = materialEditor->GetMaterial(currentCommand.materialId);
        if (ImGui::BeginCombo("Material", materialInfo ? materialInfo->filename.c_str() : "Empty Material"))
        {
            for (auto& material : materials)
            {
                if (ImGui::Selectable(material.filename.c_str(), material.resourceId == currentCommand.materialId))
                {
                    currentCommand.materialId = material.resourceId;
                    computeCommandInfo.set_material_path(material.path.c_str());
                }
            }
            ImGui::EndCombo();
        }
    }

}

bool CommandEditor::DrawContentList(bool unfocus)
{
    bool wasFocused = false;
    if (unfocus)
        currentIndex_ = commandInfos_.size();
    for (std::size_t i = 0; i < commandInfos_.size(); i++)
    {
        const auto& commandInfo = commandInfos_[i];
        if (ImGui::Selectable(commandInfo.filename.data(), currentIndex_ == i))
        {
            currentIndex_ = i;
            wasFocused = true;
        }
    }
    return wasFocused;
}

std::string_view CommandEditor::GetSubFolder()
{
    return "commands/";
}

EditorType CommandEditor::GetEditorType()
{
    return EditorType::COMMAND;
}

void CommandEditor::Save()
{
    for (auto& commandInfo : commandInfos_)
    {
        std::ofstream fileOut(commandInfo.path.c_str(), std::ios::binary);
        if (commandInfo.info.index() == 0)
        {
            if (!std::get<pb::EditorDrawCommand>(commandInfo.info).SerializeToOstream(&fileOut))
            {
                LogWarning(fmt::format("Could not save command at: {}", commandInfo.path));
            }
        }
        else
        {
            if (!std::get<pb::EditorComputeCommand>(commandInfo.info).SerializeToOstream(&fileOut))
            {
                LogWarning(fmt::format("Could not save command at: {}", commandInfo.path));
            }
        }

    }
}

CommandInfo* CommandEditor::GetCommand(ResourceId resourceId)
{
    const auto it = std::ranges::find_if(commandInfos_, [resourceId](const auto& command)
    {
       return resourceId == command.resourceId;
    });
    if(it != commandInfos_.end())
    {
        return &*it;
    }
    return nullptr;
}

void CommandEditor::ReloadId()
{
    auto* editor = Editor::GetInstance();
    const auto& resourceManager = editor->GetResourceManager();
    for (auto& commandInfo : commandInfos_)
    {
        core::Path materialPath;
        core::Path meshPath;
        core::Path bufferPath;
        if(commandInfo.info.index() == 0)
        {
            const auto& drawCommandInfo = std::get<pb::EditorDrawCommand>(commandInfo.info);
            materialPath = core::Path(drawCommandInfo.material_path());
            meshPath = core::Path(drawCommandInfo.mesh_path());
            bufferPath = core::Path(drawCommandInfo.buffer_path());
        }
        else
        {
            const auto& drawCommandInfo = std::get<pb::EditorComputeCommand>(commandInfo.info);
            materialPath = core::Path(drawCommandInfo.material_path());
        }
        if (commandInfo.materialId == INVALID_RESOURCE_ID && !materialPath.empty())
        {
            commandInfo.materialId = resourceManager.FindResourceByPath(materialPath);
        }

        if (commandInfo.meshId == INVALID_RESOURCE_ID && !meshPath.empty())
        {
            commandInfo.meshId = resourceManager.FindResourceByPath(meshPath);
        }

        if(commandInfo.bufferId == INVALID_RESOURCE_ID && !bufferPath.empty())
        {
            commandInfo.bufferId = resourceManager.FindResourceByPath(bufferPath);
        }

        UpdateMeshInCommand(std::distance(commandInfos_.data(), &commandInfo));
    }
}

void CommandEditor::Delete()
{
    if(currentIndex_ >= commandInfos_.size())
    {
        return;
    }
    auto* editor = Editor::GetInstance();
    auto& resourceManager = editor->GetResourceManager();
    resourceManager.RemoveResource(commandInfos_[currentIndex_].path, true);
}

std::span<const std::string_view> CommandEditor::GetExtensions() const
{
    static constexpr std::array<std::string_view, 2> extensions = { ".cmd", ".compcmd"};
    return std::span{ extensions };
}

void CommandEditor::Clear()
{
    commandInfos_.clear();
    currentIndex_ = -1;
}

void CommandEditor::UpdateMeshInCommand(int index)
{
    auto& currentCommand = commandInfos_[index];
    if(currentCommand.info.index() != 0)
    {
        return;
    }
    auto& drawCommandInfo = std::get<pb::EditorDrawCommand>(currentCommand.info);
    const auto* editor = Editor::GetInstance();
    auto* meshEditor = dynamic_cast<MeshEditor*>(editor->GetEditorSystem(EditorType::MESH));
    const auto* meshInfo = meshEditor->GetMesh(currentCommand.meshId);
    if (meshInfo != nullptr)
    {
        switch (meshInfo->info.mesh().primitve_type())
        {
        case core::pb::Mesh_PrimitveType_QUAD:
        {
            drawCommandInfo.mutable_draw_command()->set_draw_elements(true);
            drawCommandInfo.mutable_draw_command()->set_count(6);
            break;
        }
        case core::pb::Mesh_PrimitveType_CUBE:
        {
            drawCommandInfo.mutable_draw_command()->set_draw_elements(true);
            drawCommandInfo.mutable_draw_command()->set_count(36);
            break;
        }
        case core::pb::Mesh_PrimitveType_SPHERE:
        {
            drawCommandInfo.mutable_draw_command()->set_draw_elements(true);
            drawCommandInfo.mutable_draw_command()->set_count(core::sphereIndices);
            drawCommandInfo.mutable_draw_command()->set_mode(core::pb::DrawCommand_Mode_TRIANGLE_STRIP);
            break;
        }
        default: break;
        }
    }
}
} // namespace gpr5300
