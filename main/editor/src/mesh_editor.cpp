#include "mesh_editor.h"

namespace gpr5300
{

bool MeshEditor::CheckExtensions(std::string_view extension)
{
    static constexpr std::array<std::string_view, 3> extensions
            {
                    ".obj",
                    ".mtl",
                    ".mesh"
            };
    return std::ranges::any_of(extensions, [extension](auto modelExtension)
    {
        return extension == modelExtension;
    });
}

void MeshEditor::DrawMainView()
{

}

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
        const auto& shaderInfo = meshInfos_[i];
        if (ImGui::Selectable(shaderInfo.filename.data(), currentIndex_ == i))
        {
            currentIndex_ = i;
            wasFocused = true;
        }
    }
    return wasFocused;
}

std::string_view MeshEditor::GetSubFolder() {
    return "models/";
}

EditorType MeshEditor::GetEditorType() {
    return EditorType::MODEL;
}

void MeshEditor::Save()
{
    const auto& filesystem = FilesystemLocator::get();
    for(auto& meshInfo : meshInfos_)
    {
        filesystem.WriteString(meshInfo.path,meshInfo.info.SerializeAsString());
    }
}

void MeshEditor::AddResource(const Resource &resource)
{
    MeshInfo meshInfo{};
    meshInfo.resourceId = resource.resourceId;
    meshInfo.filename = GetFilename(resource.path);

    const auto extension = GetFileExtension(resource.path);
    if(extension == ".mesh")
    {
        const auto& fileSystem = FilesystemLocator::get();
        if (!fileSystem.IsRegularFile(resource.path))
        {
            LogWarning(fmt::format("Could not find mesh file: {}", resource.path));
            return;
        }
        const auto file = fileSystem.LoadFile(resource.path);
        if (!meshInfo.info.ParseFromString(reinterpret_cast<const char*>(file.data)))
        {
            LogWarning(fmt::format("Could not open protobuf file: {}", resource.path));
            return;
        }
    }
    else
    {
        //TODO check with obj path
        meshInfo.info.set_primitve_type(pb::Mesh_PrimitveType_MODEL);
    }
    meshInfo.path = resource.path;
    meshInfos_.push_back(meshInfo);
}

void MeshEditor::RemoveResource(const Resource &resource)
{

}

void MeshEditor::UpdateResource(const Resource &resource)
{

}
}