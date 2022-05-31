#include "command_editor.h"

#include <fstream>
#include <imgui.h>
#include <fmt/format.h>

#include "editor.h"
#include "material_editor.h"
#include "mesh_editor.h"
#include "render_pass_editor.h"
#include "engine/filesystem.h"
#include "utils/log.h"

namespace gpr5300
{
void CommandEditor::AddResource(const Resource& resource)
{
    CommandInfo commandInfo{};
    commandInfo.resourceId = resource.resourceId;
    commandInfo.filename = GetFilename(resource.path);

    const auto extension = GetFileExtension(resource.path);
    if (extension == ".cmd")
    {
        const auto& fileSystem = FilesystemLocator::get();
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
    commandInfo.info.set_name(GetFilename(resource.path, false));
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
    bool automaticDraw = currentCommand.info.automatic_draw();
    if(ImGui::Checkbox("Automatic Draw", &automaticDraw))
    {
        currentCommand.info.set_automatic_draw(automaticDraw);
    }

    UpdateMeshInCommand(currentIndex_);
    if(meshInfo != nullptr)
    {
        switch(meshInfo->info.primitve_type())
        {
        case pb::Mesh_PrimitveType_NONE:
        {
            int count = currentCommand.info.count();
            if (ImGui::InputInt("Vertex Count", &count))
            {
                currentCommand.info.set_count(count);
            }
            bool drawElements = currentCommand.info.draw_elements();
            if (ImGui::Checkbox("Draw Elements", &drawElements))
            {
                currentCommand.info.set_draw_elements(drawElements);
            }
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
    resourceManager.RemoveResource(commandInfos_[currentIndex_].path);
}

std::span<const std::string_view> CommandEditor::GetExtensions() const
{
    static constexpr std::array<std::string_view, 1> extensions = { ".cmd" };
    return std::span{ extensions };
}

void CommandEditor::UpdateMeshInCommand(int index)
{
    auto& currentCommand = commandInfos_[index];
    const auto* editor = Editor::GetInstance();
    auto* meshEditor = dynamic_cast<MeshEditor*>(editor->GetEditorSystem(EditorType::MESH));
    const auto* meshInfo = meshEditor->GetMesh(currentCommand.meshId);
    if (meshInfo != nullptr)
    {
        switch (meshInfo->info.primitve_type())
        {
        case pb::Mesh_PrimitveType_QUAD:
        {
            currentCommand.info.set_draw_elements(true);
            currentCommand.info.set_count(6);
            break;
        }
        case pb::Mesh_PrimitveType_CUBE:
        {
            currentCommand.info.set_draw_elements(true);
            currentCommand.info.set_count(36);
            break;
        }
        default: break;
        }
    }
}
} // namespace gpr5300
