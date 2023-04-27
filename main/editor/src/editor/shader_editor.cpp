#include "shader_editor.h"

#include <nlohmann/json.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>
#include "proto/renderer.pb.h"
#include "utils/log.h"
#include "engine/filesystem.h"
#include "editor.h"
#include "renderer/pipeline.h"

#include <fmt/format.h>

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

#include "scene_editor.h"

namespace py = pybind11;

// for convenience
using json = nlohmann::json;

namespace editor
{

void ShaderEditor::AddResource(const Resource& resource)
{
    ShaderInfo shaderInfo{};
    shaderInfo.info.set_type(core::GetTypeFromExtension(resource.extension));
    shaderInfo.compiledCorrectly = AnalyzeShader(resource.path, shaderInfo.info);
    shaderInfo.filename = GetFilename(resource.path);
    shaderInfo.resourceId = resource.resourceId;
    shaderInfo.info.set_path(resource.path);
    shaderInfos_.push_back(shaderInfo);
}
void ShaderEditor::RemoveResource(const Resource& resource)
{
    const auto it = std::ranges::find_if(shaderInfos_, [&resource](const auto& shaderInfo)
        {
            return resource.resourceId == shaderInfo.resourceId;
        });
    if (it != shaderInfos_.end())
    {
        shaderInfos_.erase(it);
        const auto* editor = Editor::GetInstance();
        auto* pipelineEditor = editor->GetEditorSystem(EditorType::PIPELINE);
        pipelineEditor->RemoveResource(resource);
    }
}
void ShaderEditor::UpdateExistingResource(const Resource& resource)
{
    const auto* editor = Editor::GetInstance();
    auto* pipelineEditor = editor->GetEditorSystem(EditorType::PIPELINE);
    for(auto& shaderInfo : shaderInfos_)
    {
        if(shaderInfo.resourceId == resource.resourceId)
        {
            shaderInfo.compiledCorrectly = AnalyzeShader(resource.path, shaderInfo.info);
            pipelineEditor->UpdateExistingResource(resource);
            return;
        }
    }
}
void ShaderEditor::DrawInspector()
{
    if (currentIndex_ >= shaderInfos_.size())
    {
        return;
    }

    const auto& currentShaderInfo = shaderInfos_[currentIndex_];

    ImGui::Text("Path: %s", currentShaderInfo.filename.c_str());
    if(currentShaderInfo.compiledCorrectly)
    {
        ImGui::TextColored(ImVec4(0.f, 1.0f, 0.f, 1.0f), "Shader compiled correctly");
    }
    else
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Shader failed compilation!");
    }
    switch (currentShaderInfo.info.type())
    {
    case core::pb::VERTEX:
    {
        ImGui::Text("Type: Vertex Shader");
        break;
    }
    case core::pb::FRAGMENT:
    {
        ImGui::Text("Type: Fragment Shader");
        break;
    }
    case core::pb::COMPUTE: 
        ImGui::Text("Type: Compute Shader");
        break;
    case core::pb::GEOMETRY: 
        ImGui::Text("Type: Geometry Shader");
        break;
    case core::pb::TESSELATION_CONTROL: 
        ImGui::Text("Type: Tesselation Control Shader");
        break;
    case core::pb::TESSELATION_EVAL: 
        ImGui::Text("Type: Tesselation Evaluation Shader");
        break;
    case core::pb::RAY_GEN: 
        ImGui::Text("Type: Ray Generation Shader");
        break;
    case core::pb::RAY_INTERSECTION: 
        ImGui::Text("Type: Ray Intersection Shader");
        break;
    case core::pb::RAY_ANY_HIT: 
        ImGui::Text("Type: Ray Any-Hit Shader");
        break;
    case core::pb::RAY_CLOSEST_HIT: 
        ImGui::Text("Type: Ray Closest-Hit Shader");
        break;
    case core::pb::RAY_MISS: 
        ImGui::Text("Type: Ray Miss Shader");
        break;
    case core::pb::RAY_CALL: 
        ImGui::Text("Type: Ray Callable Shader");
        break;
    default:
        break;
    }

    if(ImGui::BeginListBox("Uniforms"))
    {
        for(int i = 0; i < currentShaderInfo.info.uniforms_size(); i++)
        {
            const auto& uniformInfo = currentShaderInfo.info.uniforms(i);
            const auto text = fmt::format("Name: {} Type: {} Binding: {}", uniformInfo.name(), uniformInfo.type_name(), uniformInfo.binding());
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
    if (ImGui::BeginListBox("Structs"))
    {
        for (int i = 0; i < currentShaderInfo.info.structs_size(); i++)
        {
            const auto& structInfo = currentShaderInfo.info.structs(i);
            const auto text = fmt::format("Name: {} Size: {} Alignment: {}", structInfo.name(), structInfo.size(), structInfo.alignment());
            ImGui::Selectable(text.c_str(), false);
        }
        ImGui::EndListBox();
    }
}

void ShaderEditor::DrawCenterView()
{
    if (currentIndex_ >= shaderInfos_.size())
    {
        return;
    }

    ImGui::InputTextMultiline("Shader Content",
        &shaderText_, 
        ImGui::GetContentRegionAvail());
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
    {
        currentIndex_ = shaderInfos_.size();
        shaderText_.clear();
    }
    for (std::size_t i = 0; i < shaderInfos_.size(); i++)
    {
        const auto& shaderInfo = shaderInfos_[i];
        if (ImGui::Selectable(shaderInfo.filename.data(), currentIndex_ == i))
        {
            currentIndex_ = i;
            wasFocused = true;
            auto& filesystem = core::FilesystemLocator::get();
            std::string_view shaderPath = shaderInfo.info.path();
            if (filesystem.FileExists(shaderPath))
            {
                const auto shaderContent = filesystem.LoadFile(shaderPath);
                shaderText_ = reinterpret_cast<const char*>(shaderContent.data);
            }
            else
            {
                LogError(fmt::format("Could not load shader file: {} for central view", shaderPath));
            }
        }
    }
    return wasFocused;
}

void ShaderEditor::Save()
{
    if(currentIndex_ >= shaderInfos_.size())
    {
        return;
    }
    const auto& filesystem = core::FilesystemLocator::get();
    filesystem.WriteString(shaderInfos_[currentIndex_].info.path(), shaderText_);
    auto& resourceManager = Editor::GetInstance()->GetResourceManager();
    auto* resource = resourceManager.GetResource(shaderInfos_[currentIndex_].resourceId);
    resourceManager.UpdateExistingResource(*resource);
}

const ShaderInfo* ShaderEditor::GetShader(ResourceId resourceId) const
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

void ShaderEditor::Delete()
{
    if (currentIndex_ >= shaderInfos_.size())
    {
        return;
    }
    auto* editor = Editor::GetInstance();
    auto& resourceManager = editor->GetResourceManager();
    resourceManager.RemoveResource(shaderInfos_[currentIndex_].info.path(), true);
}

std::span<const std::string_view> ShaderEditor::GetExtensions() const
{
    static constexpr std::array<std::string_view, 11> extensions
    {
        ".vert",
        ".frag",
        ".comp",
        ".geom",
        ".tesc",
        ".tese",
        ".rchit",
        ".rahit",
        ".rgen",
        ".rmiss",
        ".rint"
    };
    return extensions;
}

void ShaderEditor::Clear()
{
    shaderInfos_.clear();
    shaderText_.clear();
    currentIndex_ = -1;
}

bool ShaderEditor::AnalyzeShader(std::string_view path, core::pb::Shader& shaderInfo) const
{
    std::string result;
    json shaderJson;
    if(GetSceneEditor()->IsVulkanScene())
    {
        py::function analyzeShaderFunc = py::module_::import("scripts.shader_parser").attr("analyze_vk_shader");
        try 
        {
            result = static_cast<py::str>(analyzeShaderFunc(path.data()));
        }
        catch (py::error_already_set& e)
        {
            LogError(fmt::format("Analyze shader failed for file: {}\n{}", path, e.what()));
        }
    }
    else
    {
        py::function analyzeShaderFunc = py::module_::import("scripts.shader_parser").attr("analyze_shader");
        try
        {
            result = static_cast<py::str>(analyzeShaderFunc(path));
        }
        catch (py::error_already_set& e)
        {
            LogError(fmt::format("Analyze shader failed for file: {}\n{}", path, e.what()));
        }
    }
    try
    {
        LogDebug(result);
        shaderJson = json::parse(result);
        int returnCode = shaderJson["returncode"].get<int>();
        if (returnCode != 0)
        {
            LogDebug(shaderJson["stdout"].get<std::string>());
            LogError(shaderJson["stderr"].get<std::string>());
            return false;
        }
        auto uniformsJson = shaderJson["uniforms"];
        shaderInfo.mutable_uniforms()->Clear();
        for(auto& uniformJson : uniformsJson)
        {
            auto uniformName = uniformJson["name"].get<std::string>();
            auto typeName = uniformJson["type_name"].get<std::string>();
            int binding = -1;
            if(uniformJson.contains("binding"))
            {
                binding = uniformJson["binding"].get<int>();
            }
            auto type = GetType(typeName);
            auto* newUniformInfo = shaderInfo.add_uniforms();
            newUniformInfo->set_name(uniformName);
            newUniformInfo->set_type(type);
            newUniformInfo->set_type_name(typeName);
            newUniformInfo->set_stage(shaderInfo.type());
            newUniformInfo->set_binding(binding);
            
        }
        shaderInfo.mutable_in_attributes()->Clear();
        auto inAttributesJson = shaderJson["in_attributes"];
        for(auto& inAttributeJson : inAttributesJson)
        {
            auto inAttributeName = inAttributeJson["name"].get<std::string>();
            auto typeName = inAttributeJson["type_name"].get<std::string>();
            auto type = GetType(typeName);
            auto* newInAttribute = shaderInfo.add_in_attributes();
            newInAttribute->set_name(inAttributeName);
            newInAttribute->set_type(type);
            newInAttribute->set_type_name(typeName);
        }
        shaderInfo.mutable_out_attributes()->Clear();
        auto& outAttributesJson = shaderJson["out_attributes"];
        for(auto& outAttributeJson : outAttributesJson)
        {
            auto outAttributeName = outAttributeJson["name"].get<std::string>();
            auto typeName = outAttributeJson["type_name"].get<std::string>();
            auto type = GetType(typeName);
            auto* newOutAttribute = shaderInfo.add_out_attributes();
            newOutAttribute->set_name(outAttributeName);
            newOutAttribute->set_type(type);
            newOutAttribute->set_type_name(typeName);
        }
        auto& structsJson = shaderJson["structs"];
        for(auto& structJson: structsJson)
        {
            auto structName = structJson["name"].get<std::string>();
            const auto size = structJson["size"].get<int>();
            const auto alignment = structJson["alignment"].get<int>();

            auto* newStruct = shaderInfo.add_structs();
            newStruct->set_name(structName);
            newStruct->set_size(size);
            newStruct->set_alignment(alignment);
            for(auto& structAttributeJson: structJson["attributes"])
            {
                auto* attribute = newStruct->add_attributes();
                attribute->set_name(structAttributeJson["name"].get<std::string>());
                attribute->set_stage(shaderInfo.type());
                auto typeName = structAttributeJson["type_name"].get<std::string>();
                auto type = GetType(typeName);
                attribute->set_type_name(typeName);
                attribute->set_type(type);
            }
        }
        return true;
    }
    catch (json::exception& e)
    {
        LogError(fmt::format("Could not parse shader info from script\n{}\n{}", e.what(), result));
    }
    return false;
}
core::pb::Attribute::Type ShaderEditor::GetType(std::string_view attibuteTypeString)
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
    const auto it = std::ranges::find(typeString, attibuteTypeString);
    if(it != typeString.end())
    {
        return static_cast<core::pb::Attribute::Type>(std::distance(typeString.begin(), it));
    }
    return core::pb::Attribute_Type_CUSTOM;
}
}