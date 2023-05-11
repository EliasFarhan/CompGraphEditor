#include "command_editor.h"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <fmt/format.h>

#include "editor.h"
#include "material_editor.h"
#include "mesh_editor.h"
#include "render_pass_editor.h"
#include "engine/filesystem.h"
#include "utils/log.h"

#include <array>
#include <fstream>

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
        if (!fileSystem.IsRegularFile(resource.path))
        {
            LogWarning(fmt::format("Could not find command file: {}", resource.path));
            return;
        }
        std::ifstream fileIn(resource.path, std::ios::binary);
        if (!commandInfo.info.ParseFromIstream(&fileIn))
        {
            LogWarning(fmt::format("Could not open protobuf file: {}", resource.path));
            return;
        }
    }
    if (commandInfo.info.draw_command().name().empty())
    {
        commandInfo.info.mutable_draw_command()->set_name(GetFilename(resource.path, false));
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
            command.info.clear_material_path();
            command.materialId = INVALID_RESOURCE_ID;
        }

        if(command.meshId == resource.resourceId)
        {
            command.info.clear_mesh_path();
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
    auto& currentCommand = commandInfos_[currentIndex_];

    //name editor
    std::string drawCommandName = currentCommand.info.draw_command().name();
    if(ImGui::InputText("Name: ", &drawCommandName))
    {
        currentCommand.info.mutable_draw_command()->set_name(drawCommandName);
    }
    
    const auto& materials = materialEditor->GetMaterials();
    const auto* materialInfo = materialEditor->GetMaterial(currentCommand.materialId);
    if(ImGui::BeginCombo("Material", materialInfo?materialInfo->filename.c_str():"Empty Material"))
    {
        for(auto& material : materials)
        {
            if(ImGui::Selectable(material.filename.c_str(), material.resourceId == currentCommand.materialId))
            {
                currentCommand.materialId = material.resourceId;
                currentCommand.info.set_material_path(material.path);
            }
        }
        ImGui::EndCombo();
    }
    const auto& meshes = meshEditor->GetMeshes();
    const auto* meshInfo = meshEditor->GetMesh(currentCommand.meshId);
    if(ImGui::BeginCombo("Mesh", meshInfo?meshInfo->filename.c_str():"Empty Mesh"))
    {
        for(auto& mesh: meshes)
        {
            if(ImGui::Selectable(mesh.filename.c_str(), mesh.resourceId == currentCommand.meshId))
            {
                currentCommand.meshId = mesh.resourceId;
                currentCommand.info.set_mesh_path(mesh.path);
            }
        }
        ImGui::EndCombo();
    }
    bool automaticDraw = currentCommand.info.draw_command().automatic_draw();
    if(ImGui::Checkbox("Automatic Draw", &automaticDraw))
    {
        currentCommand.info.mutable_draw_command()->set_automatic_draw(automaticDraw);
    }

    //Model matrix
    core::pb::Transform* transform = nullptr;
    if(currentCommand.info.draw_command().has_model_transform())
    {
        transform = currentCommand.info.mutable_draw_command()->mutable_model_transform();
    }

    if(transform == nullptr)
    {
        if(ImGui::Button("Add Model Transform Matrix"))
        {
            transform = currentCommand.info.mutable_draw_command()->mutable_model_transform();
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
        if(ImGui::InputFloat3("Position", positionTmp.data()))
        {
            position->set_x(positionTmp[0]);
            position->set_y(positionTmp[1]);
            position->set_z(positionTmp[2]);
        }
        auto* scale = transform->mutable_scale();
        std::array<float, 3> scaleTmp = { {scale->x(), scale->y(), scale->z()} };
        if(ImGui::InputFloat3("Scale", scaleTmp.data()))
        {
            scale->set_x(scaleTmp[0]);
            scale->set_y(scaleTmp[1]);
            scale->set_z(scaleTmp[2]);
        }
        auto* eulerAngles = transform->mutable_euler_angles();
        std::array<float, 3> eulerAnglesTmp = { {eulerAngles->x(), eulerAngles->y(), eulerAngles->z()} };
        if(ImGui::InputFloat3("Euler Angles", eulerAnglesTmp.data()))
        {
            eulerAngles->set_x(eulerAnglesTmp[0]);
            eulerAngles->set_y(eulerAnglesTmp[1]);
            eulerAngles->set_z(eulerAnglesTmp[2]);
        }
        if(ImGui::Button("Remove Model Transform Matrix"))
        {
            currentCommand.info.mutable_draw_command()->clear_model_transform();
        }
    }

    UpdateMeshInCommand(currentIndex_);
    if(meshInfo != nullptr)
    {
        switch(meshInfo->info.mesh().primitve_type())
        {
        case core::pb::Mesh_PrimitveType_NONE:
        {
            int count = currentCommand.info.mutable_draw_command()->count();
            if (ImGui::InputInt("Vertex Count", &count))
            {
                currentCommand.info.mutable_draw_command()->set_count(count);
            }
            bool drawElements = currentCommand.info.mutable_draw_command()->draw_elements();
            if (ImGui::Checkbox("Draw Elements", &drawElements))
            {
                currentCommand.info.mutable_draw_command()->set_draw_elements(drawElements);
            }

            break;
        }
        case core::pb::Mesh_PrimitveType_MODEL:
        {
            ImGui::Text("Vertex Count: %d", currentCommand.info.draw_command().count());
            break;
        }
        default:
            break;
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
        std::ofstream fileOut(commandInfo.path, std::ios::binary);
        if (!commandInfo.info.SerializeToOstream(&fileOut))
        {
            LogWarning(fmt::format("Could not save command at: {}", commandInfo.path));
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
        if (commandInfo.materialId == INVALID_RESOURCE_ID && !commandInfo.info.material_path().empty())
        {
            commandInfo.materialId = resourceManager.FindResourceByPath(commandInfo.info.material_path());
        }

        if (commandInfo.meshId == INVALID_RESOURCE_ID && !commandInfo.info.mesh_path().empty())
        {
            commandInfo.meshId = resourceManager.FindResourceByPath(commandInfo.info.mesh_path());
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
    static constexpr std::array<std::string_view, 1> extensions = { ".cmd" };
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
    const auto* editor = Editor::GetInstance();
    auto* meshEditor = dynamic_cast<MeshEditor*>(editor->GetEditorSystem(EditorType::MESH));
    const auto* meshInfo = meshEditor->GetMesh(currentCommand.meshId);
    if (meshInfo != nullptr)
    {
        switch (meshInfo->info.mesh().primitve_type())
        {
        case core::pb::Mesh_PrimitveType_QUAD:
        {
            currentCommand.info.mutable_draw_command()->set_draw_elements(true);
            currentCommand.info.mutable_draw_command()->set_count(6);
            break;
        }
        case core::pb::Mesh_PrimitveType_CUBE:
        {
            currentCommand.info.mutable_draw_command()->set_draw_elements(true);
            currentCommand.info.mutable_draw_command()->set_count(36);
            break;
        }
        case core::pb::Mesh_PrimitveType_SPHERE:
        {
            currentCommand.info.mutable_draw_command()->set_draw_elements(true);
            currentCommand.info.mutable_draw_command()->set_count(core::sphereIndices);
            currentCommand.info.mutable_draw_command()->set_mode(core::pb::DrawCommand_Mode_TRIANGLE_STRIP);
            break;
        }
        default: break;
        }
    }
}
} // namespace gpr5300
