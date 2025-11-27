#include "command_editor.h"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <format>

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
#include "utils/fb_file.h"

namespace novus::editor
{
void CommandEditor::AddResource(const Resource& resource)
{
    CommandInfo commandInfo{};
    commandInfo.resourceId = resource.resourceId;
    commandInfo.filename = GetFilename(resource.path);

    const auto extension = GetFileExtension(resource.path);
    if (extension == ".cmd")
    {
        if (!core::IsRegularFile(resource.path.c_str()))
        {
            LogWarning(std::format("Could not find command file: {}", resource.path.c_str()));
            return;
        }
        std::ifstream fileIn(resource.path.c_str(), std::ios::binary);
        commandInfo.info.emplace<EditorDrawCommandInfoT>();
        auto& info = std::get<EditorDrawCommandInfoT>(commandInfo.info);
        if (!core::ReadFlatbufferFromFile<EditorDrawCommandInfoT, EditorDrawCommandInfo>(resource.path, info))
        {
            LogWarning(std::format("Could not open protobuf file: {}", resource.path.c_str()));
            return;
        }
        if (info.draw_command->name.empty())
        {
            info.draw_command->name = (GetFilename(resource.path, false));
        }
    }
    else if(extension ==".compcmd")
    {
        //TODO load compute command
        /*
        if (!core::IsRegularFile(resource.path.c_str()))
        {
            LogWarning(std::format("Could not find command file: {}", resource.path.c_str()));
            return;
        }
        std::ifstream fileIn(resource.path.c_str(), std::ios::binary);
        commandInfo.info.emplace<pb::EditorComputeCommand>();
        auto& info = std::get<pb::EditorComputeCommand>(commandInfo.info);
        if (!info.ParseFromIstream(&fileIn))
        {
            LogWarning(std::format("Could not open protobuf file: {}", resource.path.c_str()));
            return;
        }
        if (info.compute_command().name().empty())
        {
            info.mutable_compute_command()->set_name(GetFilename(resource.path, false));
        }
        */
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
                std::get<EditorDrawCommandInfoT>(command.info).material_path.clear();
            }
            else
            {
                //TODO clear material path in compute command
                //std::get<pb::EditorComputeCommand>(command.info).clear_material_path();
            }
            command.materialId = INVALID_RESOURCE_ID;
        }

        if(command.meshId == resource.resourceId)
        {
            if (command.info.index() == 0)
            {
                std::get<EditorDrawCommandInfoT>(command.info).mesh_path.clear();
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
    if (currentCommand.info.index() == 0) // if command is graphics
    {
        auto& drawCommandInfo = std::get<EditorDrawCommandInfoT>(currentCommand.info);
        if (ImGui::InputText("Name: ", &drawCommandInfo.draw_command->name))
        {
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
                    drawCommandInfo.material_path = (material.path);
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
                    drawCommandInfo.mesh_path = (mesh.path);
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
            pipelineInfo->computeShaderId
        };
        //TODO ssbo binding list?
        /*
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
            std::string_view currentBufferPath{drawCommandInfo.buffer_path()};

            if (ImGui::BeginCombo("Storage Buffer", currentBufferPath.empty() ? "No Buffer" : currentBufferPath.data()))
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
        */

        {
            bool automaticDraw = drawCommandInfo.draw_command->automatic_draw;
            if (ImGui::Checkbox("Automatic Draw", &drawCommandInfo.draw_command->automatic_draw))
            {
            }

            //TODO add Model matrix to command
            /*
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
            */

            UpdateMeshInCommand(currentIndex_);
            if (meshInfo != nullptr)
            {
                switch (meshInfo->info.mesh->primitive_type)
                {
                case renderer::MeshPrimitiveType_NONE:
                {
                    if (ImGui::InputInt("Vertex Count", &drawCommandInfo.draw_command->count))
                    {
                    }
                    if (ImGui::Checkbox("Draw Elements", &drawCommandInfo.draw_command->draw_elements))
                    {
                    }

                    break;
                }
                case renderer::MeshPrimitiveType_MODEL:
                {
                    ImGui::Text("Vertex Count: %d", drawCommandInfo.draw_command->count);
                    break;
                }
                default:
                    break;
                }
            }
        }

    }
    else
    {
        /*
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
        */
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
            if (!core::WriteFlatbufferToFile<EditorDrawCommandInfoT, EditorDrawCommandInfo>(std::get<EditorDrawCommandInfoT>(commandInfo.info), commandInfo.path))
            {
                LogWarning(std::format("Could not save command at: {}", commandInfo.path));
            }
        }
        else
        {
            //TODO saving compute command
            /*
            if (!std::get<pb::EditorComputeCommand>(commandInfo.info).SerializeToOstream(&fileOut))
            {
                LogWarning(std::format("Could not save command at: {}", commandInfo.path.c_str()));
            }
            */
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
        std::string materialPath;
        std::string meshPath;
        std::string bufferPath;
        if(commandInfo.info.index() == 0)
        {
            const auto& drawCommandInfo = std::get<EditorDrawCommandInfoT>(commandInfo.info);
            materialPath = drawCommandInfo.material_path;
            meshPath = drawCommandInfo.mesh_path;
            //TODO why buffer for a command?
            //bufferPath = drawCommandInfo.buffer_path; //buffer?
        }
        else
        {
            //TODO compute material path
            /*
            const auto& drawCommandInfo = std::get<pb::EditorComputeCommand>(commandInfo.info);
            materialPath = drawCommandInfo.material_path();
            */
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
    auto& drawCommandInfo = std::get<EditorDrawCommandInfoT>(currentCommand.info);
    const auto* editor = Editor::GetInstance();
    auto* meshEditor = dynamic_cast<MeshEditor*>(editor->GetEditorSystem(EditorType::MESH));
    const auto* meshInfo = meshEditor->GetMesh(currentCommand.meshId);
    if (meshInfo != nullptr)
    {
        switch (meshInfo->info.mesh->primitive_type)
        {
        case renderer::MeshPrimitiveType_QUAD:
        {
            drawCommandInfo.draw_command->draw_elements = (true);
            drawCommandInfo.draw_command->count = (6);
            break;
        }
        case renderer::MeshPrimitiveType_CUBE:
        {
            drawCommandInfo.draw_command->draw_elements = (true);
            drawCommandInfo.draw_command->count = (36);
            break;
        }
        case renderer::MeshPrimitiveType_SPHERE:
        {
            drawCommandInfo.draw_command->draw_elements = (true);
            drawCommandInfo.draw_command->count = (core::sphereIndices);
            drawCommandInfo.draw_command->mode = renderer::DrawMode_TRIANGLE_STRIP;
            break;
        }
        default: break;
        }
    }
}
} // namespace gpr5300
