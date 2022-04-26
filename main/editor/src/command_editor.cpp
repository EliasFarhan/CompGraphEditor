#include "command_editor.h"

#include <fstream>
#include <imgui.h>
#include <fmt/format.h>

#include "editor.h"
#include "material_editor.h"
#include "mesh_editor.h"
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
    commandInfo.path = resource.path;
    commandInfos_.push_back(commandInfo);
}

void CommandEditor::RemoveResource(const Resource& resource)
{
}

void CommandEditor::UpdateExistingResource(const Resource& resource)
{
}

bool CommandEditor::CheckExtensions(std::string_view extension)
{
    return extension == ".cmd";
}

void CommandEditor::DrawMainView()
{
}

void CommandEditor::DrawInspector()
{
    if (currentIndex_ >= commandInfos_.size())
    {
        return;
    }
    auto* editor = Editor::GetInstance();
    const auto& resourceManager = editor->GetResourceManager();
    auto* materialEditor = dynamic_cast<MaterialEditor*>(editor->GetEditorSystem(EditorType::MATERIAL));
    auto* meshEditor = dynamic_cast<MeshEditor*>(editor->GetEditorSystem(EditorType::MODEL));
    auto& currentCommand = commandInfos_[currentIndex_];

    if(currentCommand.materialId == INVALID_RESOURCE_ID && !currentCommand.info.material_path().empty())
    {
        currentCommand.materialId = resourceManager.FindResourceByPath(currentCommand.info.material_path());
    }

    if(currentCommand.meshId == INVALID_RESOURCE_ID && !currentCommand.info.mesh_path().empty())
    {
        currentCommand.meshId = resourceManager.FindResourceByPath(currentCommand.info.mesh_path());
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
    bool automaticDraw = currentCommand.info.automatic_draw();
    if(ImGui::Checkbox("Automatic Draw", &automaticDraw))
    {
        currentCommand.info.set_automatic_draw(automaticDraw);
    }

    if(meshInfo != nullptr)
    {
        switch(meshInfo->info.primitve_type())
        {
        case pb::Mesh_PrimitveType_QUAD:
        {
            currentCommand.info.set_draw_elements(true);
            currentCommand.info.set_count(6);
            break;
        }
        case pb::Mesh_PrimitveType_CUBE: break;
        case pb::Mesh_PrimitveType_SPHERE: break;
        case pb::Mesh_PrimitveType_MODEL: break;
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
        default: break;
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
        std::ofstream fileOut(commandInfo.path, std::ios::binary, std::ios::binary);
        if (!commandInfo.info.SerializeToOstream(&fileOut))
        {
            LogWarning(fmt::format("Could not save command at: {}", commandInfo.path));
        }

    }
}
} // namespace gpr5300
