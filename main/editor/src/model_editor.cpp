#include "model_editor.h"
#include <array>
#include "utils/log.h"
#include <fmt/format.h>
#include <tiny_obj_loader.h>

namespace gpr5300
{
void ModelEditor::AddResource(const Resource& resource)
{
    tinyobj::ObjReaderConfig readerConfig{};
    readerConfig.triangulate = true;
    readerConfig.vertex_color = false;
    tinyobj::ObjReader reader;

    if(!reader.ParseFromFile(resource.path, readerConfig))
    {
        if(!reader.Error().empty())
        {
            LogError(fmt::format("Error parsing obj file: {}", resource.path));
        }
    }

    auto& shapes = reader.GetShapes();
    LogDebug(fmt::format("Newly imported model containes {} meshes", shapes.size()));
    for(auto& shape : shapes)
    {
        LogDebug(fmt::format("Mesh: {} contains {} vertices", shape.name, shape.mesh.indices.size()));
    }
}

void ModelEditor::RemoveResource(const Resource& resource)
{
}

void ModelEditor::UpdateExistingResource(const Resource& resource)
{
}


void ModelEditor::DrawMainView()
{
}

void ModelEditor::DrawInspector()
{
}

bool ModelEditor::DrawContentList(bool unfocus)
{
    return false;
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
}

void ModelEditor::ReloadId()
{
}

void ModelEditor::Delete()
{
}

std::span<const std::string_view> ModelEditor::GetExtensions() const
{
    static constexpr std::array<std::string_view, 1> extensions = { ".obj" };
    return std::span{ extensions };
}
}
