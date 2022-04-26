#include "material_editor.h"

namespace gpr5300
{

bool MaterialEditor::CheckExtensions(std::string_view extension)
{
    return extension == ".mat";
}

void MaterialEditor::DrawMainView()
{

}

void MaterialEditor::DrawInspector()
{
    if (currentIndex_ == materialInfos_.size())
    {
        return;
    }
}

bool MaterialEditor::DrawContentList(bool unfocus) {
    bool wasFocused = false;
    if (unfocus)
        currentIndex_ = materialInfos_.size();
    for (std::size_t i = 0; i < materialInfos_.size(); i++)
    {
        const auto& shaderInfo = materialInfos_[i];
        if (ImGui::Selectable(shaderInfo.filename.data(),  currentIndex_ == i))
        {
            currentIndex_ = i;
            wasFocused = true;
        }
    }
    return wasFocused;
}

std::string_view MaterialEditor::GetSubFolder() {
    return "materials/";
}

EditorType MaterialEditor::GetEditorType() {
    return EditorType::MATERIAL;
}

void MaterialEditor::Save()
{
    const auto& filesystem = FilesystemLocator::get();
    for(auto& materialInfo : materialInfos_)
    {
        filesystem.WriteString(materialInfo.path,materialInfo.info.SerializeAsString());
    }
}

void MaterialEditor::AddResource(const Resource &resource)
{
    MaterialInfo materialInfo{};
    materialInfo.filename = GetFilename(resource.path);
    materialInfo.resourceId = resource.resourceId;
    materialInfo.path = resource.path;
    const auto& fileSystem = FilesystemLocator::get();

    if (!fileSystem.IsRegularFile(resource.path))
    {
        LogWarning(fmt::format("Could not find material file: {}", resource.path));
        return;
    }
    const auto file = fileSystem.LoadFile(resource.path);
    if (!materialInfo.info.ParseFromString(reinterpret_cast<const char*>(file.data)))
    {
        LogWarning(fmt::format("Could not open protobuf file: {}", resource.path));
        return;
    }
    materialInfos_.push_back(materialInfo);
}

void MaterialEditor::RemoveResource(const Resource &resource)
{

}

void MaterialEditor::UpdateResource(const Resource &resource)
{

}
}