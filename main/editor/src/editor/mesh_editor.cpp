#include "mesh_editor.h"
#include "engine/filesystem.h"
#include "utils/log.h"
#include "editor.h"
#include "command_editor.h"

#include <imgui.h>
#include <format>

#include <string_view>
#include <algorithm>
#include <array>
#include <fstream>

#include "utils/fb_file.h"

namespace novus::editor
{

void MeshEditor::DrawInspector()
{
    if(currentIndex_ >= meshInfos_.size())
    {
        return;
    }

    auto& currentMesh = meshInfos_[currentIndex_];

    if(currentMesh.info.mesh->primitive_type == renderer::MeshPrimitiveType_MODEL)
    {
        const auto modelName = GetFilename(currentMesh.info.model_path);
        ImGui::Text("Generated from model: %s", modelName.c_str());
        return;
    }

    bool none = currentMesh.info.mesh->primitive_type == renderer::MeshPrimitiveType_NONE;
    if(ImGui::Checkbox("No Mesh", &none))
    {
        if(none)
        {
            currentMesh.info.mesh->primitive_type = renderer::MeshPrimitiveType_NONE;
        }
    }
    bool primitive = !none && currentMesh.info.mesh->primitive_type != renderer::MeshPrimitiveType_MODEL;
    if(ImGui::Checkbox("Primitive", &primitive))
    {
        if((primitive && none) ||
            (primitive && currentMesh.info.mesh->primitive_type == renderer::MeshPrimitiveType_MODEL))
        {
            currentMesh.info.mesh->primitive_type = renderer::MeshPrimitiveType_QUAD;
        }
        if(!primitive)
        {
            currentMesh.info.mesh->primitive_type = renderer::MeshPrimitiveType_NONE;
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
        int index = currentMesh.info.mesh->primitive_type;
        if(index < 0 || index >= primitiveTypes.size())
        {
            index = 0;
            currentMesh.info.mesh->primitive_type = renderer::MeshPrimitiveType_QUAD;
        }
        if(ImGui::BeginCombo("Primitive Type", primitiveTypes[index].data()))
        {
            for(std::size_t i = 0; i < primitiveTypes.size(); i++)
            {
                if(ImGui::Selectable(primitiveTypes[i].data(), i == index))
                {
                    currentMesh.info.mesh->primitive_type = (renderer::MeshPrimitiveType)(renderer::MeshPrimitiveType_QUAD + i);
                }
            }
            ImGui::EndCombo();
        }
        if(currentMesh.info.mesh->primitive_type == renderer::MeshPrimitiveType_QUAD ||
            currentMesh.info.mesh->primitive_type == renderer::MeshPrimitiveType_CUBE ||
            currentMesh.info.mesh->primitive_type == renderer::MeshPrimitiveType_SPHERE)
        {
            if(currentMesh.info.mesh->scale.x == 0.0f)
            {
                currentMesh.info.mesh->scale = {1,1,1};
            }
            auto* scale = &currentMesh.info.mesh->scale[0];
            if(ImGui::InputFloat3("Scale", scale))
            {
            }

            auto* offset = &currentMesh.info.mesh->offset[0];
            if(ImGui::InputFloat3("Offset", offset))
            {
            }
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

std::string_view MeshEditor::GetSubFolder()
{
    return "meshes/";
}

EditorType MeshEditor::GetEditorType()
{
    return EditorType::MESH;
}

void MeshEditor::Save()
{
    for(auto& meshInfo : meshInfos_)
    {
        std::ofstream fileOut(meshInfo.path.c_str(), std::ios::binary);
        if (!core::WriteFlatbufferToFile<EditorMeshInfoT, EditorMeshInfo>(meshInfo.info, meshInfo.path))
        {
            LogWarning(std::format("Could not save mesh at: {}", meshInfo.path));
        }
        
    }
}

void MeshEditor::AddResource(const Resource &resource)
{
    MeshInfo meshInfo{};
    meshInfo.resourceId = resource.resourceId;
    meshInfo.filename = GetFilename(resource.path);

    const auto extension = GetFileExtension(resource.path);

    if (!core::IsRegularFile(resource.path.c_str()))
    {
        LogWarning(std::format("Could not find mesh file: {}", resource.path.c_str()));
        return;
    }
    std::ifstream fileIn(resource.path.c_str(), std::ios::binary);
    if (!core::ReadFlatbufferFromFile<EditorMeshInfoT, EditorMeshInfo>(resource.path, meshInfo.info))
    {
        LogWarning(std::format("Could not open protobuf file: {}", resource.path.c_str()));
        return;
    }
    
    meshInfo.path = resource.path;
    meshInfos_.push_back(meshInfo);
}

void MeshEditor::RemoveResource(const Resource& resource)
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
    resourceManager.RemoveResource(meshInfos_[currentIndex_].path, true);
}

std::span<const std::string_view> MeshEditor::GetExtensions() const
{
    static constexpr std::array<std::string_view, 1> extensions = { ".mesh" };
    return std::span{ extensions };
}

void MeshEditor::Clear()
{
    meshInfos_.clear();
    currentIndex_ = -1;
}
}
