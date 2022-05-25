#include "mesh_editor.h"
#include "engine/filesystem.h"
#include "utils/log.h"
#include "editor.h"
#include "command_editor.h"
#include <imgui.h>
#include <fmt/format.h>
#include <string_view>
#include <algorithm>
#include <array>
#include <fstream>

namespace gpr5300
{

void MeshEditor::DrawInspector()
{
    if(currentIndex_ >= meshInfos_.size())
    {
        return;
    }
    auto& currentMesh = meshInfos_[currentIndex_];

    bool none = currentMesh.info.primitve_type() == pb::Mesh_PrimitveType_NONE;
    if(ImGui::Checkbox("No Mesh", &none))
    {
        if(none)
        {
            currentMesh.info.set_primitve_type(pb::Mesh_PrimitveType_NONE);
        }
    }
    bool primitive = !none && currentMesh.info.primitve_type() != pb::Mesh_PrimitveType_MODEL;
    if(ImGui::Checkbox("Primitive", &primitive))
    {
        if((primitive && none) ||
            (primitive && currentMesh.info.primitve_type() == pb::Mesh_PrimitveType_MODEL))
        {
            currentMesh.info.set_primitve_type(pb::Mesh_PrimitveType_QUAD);
        }
        if(!primitive)
        {
            currentMesh.info.set_primitve_type(pb::Mesh_PrimitveType_NONE);
        }
    }
    if(primitive)
    {
        static constexpr std::array<std::string_view, 3> primitiveTypes =
                {
                        "Quad",
                        "Cube",
                        "Sphere"
                };
        auto index = currentMesh.info.primitve_type()-pb::Mesh_PrimitveType_QUAD;
        if(index < 0 || index >= primitiveTypes.size())
        {
            index = 0;
            currentMesh.info.set_primitve_type(pb::Mesh_PrimitveType_QUAD);
        }
        if(ImGui::BeginCombo("Primitive Type", primitiveTypes[index].data()))
        {
            for(std::size_t i = 0; i < primitiveTypes.size(); i++)
            {
                if(ImGui::Selectable(primitiveTypes[i].data(), i == index))
                {
                    currentMesh.info.set_primitve_type(
                            static_cast<pb::Mesh_PrimitveType>(pb::Mesh_PrimitveType_QUAD + i));
                }
            }
            ImGui::EndCombo();
        }
    }
}

bool MeshEditor::DrawContentList(bool unfocus)
{
    bool wasFocused = false;
    if (unfocus)
        currentIndex_ = meshInfos_.size();
    for (std::size_t i = 0; i < meshInfos_.size(); i++)
    {
        const auto& meshInfo = meshInfos_[i];
        if (ImGui::Selectable(meshInfo.filename.data(), currentIndex_ == i))
        {
            currentIndex_ = i;
            wasFocused = true;
        }
    }
    return wasFocused;
}

std::string_view MeshEditor::GetSubFolder() {
    return "meshes/";
}

EditorType MeshEditor::GetEditorType() {
    return EditorType::MESH;
}

void MeshEditor::Save()
{
    for(auto& meshInfo : meshInfos_)
    {
        std::ofstream fileOut(meshInfo.path, std::ios::binary);
        if (!meshInfo.info.SerializeToOstream(&fileOut))
        {
            LogWarning(fmt::format("Could not save mesh at: {}", meshInfo.path));
        }
        
    }
}

void MeshEditor::AddResource(const Resource &resource)
{
    MeshInfo meshInfo{};
    meshInfo.resourceId = resource.resourceId;
    meshInfo.filename = GetFilename(resource.path);

    const auto extension = GetFileExtension(resource.path);

    const auto& fileSystem = FilesystemLocator::get();
    if (!fileSystem.IsRegularFile(resource.path))
    {
        LogWarning(fmt::format("Could not find mesh file: {}", resource.path));
        return;
    }
    std::ifstream fileIn(resource.path, std::ios::binary);
    if (!meshInfo.info.ParseFromIstream(&fileIn))
    {
        LogWarning(fmt::format("Could not open protobuf file: {}", resource.path));
        return;
    }
    
    meshInfo.path = resource.path;
    meshInfos_.push_back(meshInfo);
}

void MeshEditor::RemoveResource(const Resource &resource)
{
    const auto it = std::ranges::find_if(meshInfos_, [&resource](const auto& mesh)
        {
            return resource.resourceId == mesh.resourceId;
        });
    if(it != meshInfos_.end())
    {
        meshInfos_.erase(it);
        const auto* editor = Editor::GetInstance();
        auto* commandEditor = dynamic_cast<CommandEditor*>(editor->GetEditorSystem(EditorType::COMMAND));
        commandEditor->RemoveResource(resource);
    }
}

void MeshEditor::UpdateExistingResource(const Resource &resource)
{

}

MeshInfo* MeshEditor::GetMesh(ResourceId resourceId)
{
    const auto it = std::ranges::find_if(meshInfos_, [resourceId](const auto& meshInfo)
        {
            return resourceId == meshInfo.resourceId;
        });
    if(it != meshInfos_.end())
    {
        return &*it;
    }
    return nullptr;
}

void MeshEditor::Delete()
{
    if (currentIndex_ >= meshInfos_.size())
    {
        return;
    }
    auto* editor = Editor::GetInstance();
    auto& resourceManager = editor->GetResourceManager();
    resourceManager.RemoveResource(meshInfos_[currentIndex_].path);
}

std::span<const std::string_view> MeshEditor::GetExtensions() const
{
    static constexpr std::array<std::string_view, 1> extensions = { ".mesh" };
    return std::span{ extensions };
}
}
