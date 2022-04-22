#include "shader_editor.h"

namespace gpr5300
{

void ShaderEditor::AddResource(const Resource& resource)
{
    py::function analyzeShaderFunc = py::module_::import("scripts.shader_parser").attr("analyze_shader");
    try
    {
        std::string result = (py::str) analyzeShaderFunc(resource.path);
        LogDebug(fmt::format("Loading shader: {} with content:\n{}", resource.path, result));
    }
    catch (py::error_already_set& e)
    {
        LogError(fmt::format("Analyze shader failed\n{}", e.what()));
    }
    ShaderInfo shaderInfo{};
    shaderInfo.filename = GetFilename(resource.path);
    shaderInfo.resourceId = resource.resourceId;
    shaderInfo.info.set_path(resource.path);
    if(resource.extension == ".vert")
    {
        shaderInfo.info.set_type(pb::Shader_Type_VERTEX);
    }
    else if(resource.extension == ".frag")
    {
        shaderInfo.info.set_type(pb::Shader_Type_FRAGMENT);
    }
    shaderInfos_.push_back(shaderInfo);
}
void ShaderEditor::RemoveResource(const Resource &resource)
{

}
void ShaderEditor::UpdateResource(const Resource &resource)
{

}
bool ShaderEditor::CheckExtensions(std::string_view extension)
{
    static constexpr std::array<std::string_view, 3> extensions
        {
            ".vert",
            ".frag",
            ".comp"
        };
    return std::ranges::any_of(extensions, [extension](auto shaderExtension){
        return extension == shaderExtension;
    });
}
void ShaderEditor::DrawMainView()
{

}
void ShaderEditor::DrawInspector()
{
    ImGui::Text("Shader Inspector");
}
std::string_view ShaderEditor::GetSubFolder()
{
    return "shaders/";
}
EditorType ShaderEditor::GetEditorType()
{
    return EditorType::SHADER;
}
bool ShaderEditor::DrawContentList(bool unfocus)
{
    bool wasFocused = false;
    if(unfocus)
        currentIndex_ = shaderInfos_.size();
    for(std::size_t i = 0; i < shaderInfos_.size(); i++)
    {
        const auto& shaderInfo = shaderInfos_[i];
        if(ImGui::Selectable(shaderInfo.filename.data(), currentIndex_ == i))
        {
            currentIndex_ = i;
            wasFocused = true;
        }
    }
    return wasFocused;
}
}