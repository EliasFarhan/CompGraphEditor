#include "shader_editor.h"

#include <nlohmann/json.hpp>

#include <imgui.h>
#include "proto/renderer.pb.h"
#include "utils/log.h"
#include <fmt/format.h>

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

// for convenience
using json = nlohmann::json;

namespace gpr5300
{

void ShaderEditor::AddResource(const Resource& resource)
{

    ShaderInfo shaderInfo{};
    if(!AnalyzeShader(resource.path, shaderInfo.info))
    {
        return;
    }
    shaderInfo.filename = GetFilename(resource.path);
    shaderInfo.resourceId = resource.resourceId;
    shaderInfo.info.set_path(resource.path);
    if (resource.extension == ".vert")
    {
        shaderInfo.info.set_type(pb::Shader_Type_VERTEX);
    }
    else if (resource.extension == ".frag")
    {
        shaderInfo.info.set_type(pb::Shader_Type_FRAGMENT);
    }
    shaderInfos_.push_back(shaderInfo);
}
void ShaderEditor::RemoveResource(const Resource& resource)
{

}
void ShaderEditor::UpdateResource(const Resource& resource)
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
    return std::ranges::any_of(extensions, [extension](auto shaderExtension)
    {
        return extension == shaderExtension;
    });
}
void ShaderEditor::DrawMainView()
{

}
void ShaderEditor::DrawInspector()
{
    if (currentIndex_ >= shaderInfos_.size())
    {
        return;
    }

    const auto& currentShaderInfo = shaderInfos_[currentIndex_];

    ImGui::Text("Path: %s", currentShaderInfo.filename.c_str());
    switch (currentShaderInfo.info.type())
    {
    case pb::Shader_Type_VERTEX:
    {
        ImGui::Text("Type: Vertex Shader");
        break;
    }
    case pb::Shader_Type_FRAGMENT:
    {
        ImGui::Text("Type: Fragment Shader");
        break;
    }
    default:
        break;
    }

    if(ImGui::BeginListBox("Uniforms"))
    {
        for(int i = 0; i < currentShaderInfo.info.uniforms_size(); i++)
        {
            const auto& uniformInfo = currentShaderInfo.info.uniforms(i);
            const auto text = fmt::format("Name: {} Type: {}", uniformInfo.name(), uniformInfo.type_name());
            ImGui::Selectable(text.c_str(), false);
        }
        ImGui::EndListBox();
    }

    if(ImGui::BeginListBox("In Attributes"))
    {
        for(int i = 0; i < currentShaderInfo.info.in_attributes_size(); i++)
        {
            const auto& inAttributeInfo = currentShaderInfo.info.in_attributes(i);
            const auto text = fmt::format("Name: {} Type: {}", inAttributeInfo.name(), inAttributeInfo.type_name());
            ImGui::Selectable(text.c_str(), false);
        }
        ImGui::EndListBox();
    }

    if(ImGui::BeginListBox("Out Attributes"))
    {
        for(int i = 0; i < currentShaderInfo.info.out_attributes_size(); i++)
        {
            const auto& outAttributeInfo = currentShaderInfo.info.out_attributes(i);
            const auto text = fmt::format("Name: {} Type: {}", outAttributeInfo.name(), outAttributeInfo.type_name());
            ImGui::Selectable(text.c_str(), false);
        }
        ImGui::EndListBox();
    }
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
    if (unfocus)
        currentIndex_ = shaderInfos_.size();
    for (std::size_t i = 0; i < shaderInfos_.size(); i++)
    {
        const auto& shaderInfo = shaderInfos_[i];
        if (ImGui::Selectable(shaderInfo.filename.data(), currentIndex_ == i))
        {
            currentIndex_ = i;
            wasFocused = true;
        }
    }
    return wasFocused;
}

const ShaderInfo* ShaderEditor::GetShader(ResourceId resourceId)
{
    if(resourceId == INVALID_RESOURCE_ID)
    {
        return nullptr;
    }
    const auto it = std::ranges::find_if(shaderInfos_, [resourceId](const auto& shader)
    {
        return resourceId == shader.resourceId;
    });
    if(it != shaderInfos_.end())
    {
        return &*it;
    }
    return nullptr;
}

bool ShaderEditor::AnalyzeShader(std::string_view path, pb::Shader& shaderInfo)
{
    py::function analyzeShaderFunc = py::module_::import("scripts.shader_parser").attr("analyze_shader");
    try
    {
        std::string result = static_cast<py::str>(analyzeShaderFunc(path));
        LogDebug(fmt::format("Loading shader: {} with content:\n{}", path, result));
        auto shaderJson = json::parse(result);

        auto uniformsJson = shaderJson["uniforms"];
        for(auto& uniformJson : uniformsJson)
        {
            auto uniformName = uniformJson["name"].get<std::string>();
            auto typeName = uniformJson["type"].get<std::string>();
            auto type = GetType(typeName);
            auto* newUniformInfo = shaderInfo.add_uniforms();
            newUniformInfo->set_name(uniformName);
            newUniformInfo->set_type(type);
            newUniformInfo->set_type_name(typeName);
        }
        auto inAttributesJson = shaderJson["in_attributes"];
        for(auto& inAttributeJson : inAttributesJson)
        {
            auto inAttributeName = inAttributeJson["name"].get<std::string>();
            auto typeName = inAttributeJson["type"].get<std::string>();
            auto type = GetType(typeName);
            auto* newInAttribute = shaderInfo.add_in_attributes();
            newInAttribute->set_name(inAttributeName);
            newInAttribute->set_type(type);
            newInAttribute->set_type_name(typeName);
        }

        auto outAttributesJson = shaderJson["out_attributes"];
        for(auto& outAttributeJson : outAttributesJson)
        {
            auto outAttributeName = outAttributeJson["name"].get<std::string>();
            auto typeName = outAttributeJson["type"].get<std::string>();
            auto type = GetType(typeName);
            auto* newOutAttribute = shaderInfo.add_out_attributes();
            newOutAttribute->set_name(outAttributeName);
            newOutAttribute->set_type(type);
            newOutAttribute->set_type_name(typeName);
        }
        return true;
    }
    catch (py::error_already_set& e)
    {
        LogError(fmt::format("Analyze shader failed for file: {}\n{}", path, e.what()));
    }
    catch (json::parse_error& e)
    {
        LogError(fmt::format("Could not parse shader info from script\n{}", e.what()));
    }
    return false;
}
pb::Attribute::Type ShaderEditor::GetType(std::string_view attibuteTypeString)
{
    static constexpr std::array<std::string_view, 15> typeString =
        {
        "float",
        "vec2",
        "vec3",
        "vec4",
        "mat2",
        "mat3",
        "mat4",
        "int",
        "ivec2",
        "ivec3",
        "ivec4",
        "bool",
        "sampler2D",
        "samplerCube",
        "void"
        };
    auto it = std::ranges::find(typeString, attibuteTypeString);
    if(it != typeString.end())
    {
        return (pb::Attribute::Type)std::distance(typeString.begin(), it);
    }
    return pb::Attribute_Type_CUSTOM;
}
}