#include "shader_editor.h"

#include <nlohmann/json.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>
#include "utils/log.h"
#include "engine/filesystem.h"
#include "editor.h"
#include "renderer/pipeline.h"

#include <format>

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

#include "../../../utils/shader_analyzer_lib/shader_analyzer.h"
#include "scene_editor.h"

namespace py = pybind11;

// for convenience
using json = nlohmann::json;

namespace novus::editor
{
bool CheckVertexInput(const renderer::ShaderT& shaderInfo)
{
    return true;
    // TODO check vertex inputs according to the engine
    /*
    if (shaderInfo.type() != core::pb::VERTEX)
        return true;
    for(int i = 0; i < shaderInfo.in_attributes_size(); i++)
    {
        switch (shaderInfo.in_attributes(i).location())
        {
        case 0:
        case 2:
        case 3:
        case 4:
        {
            if (shaderInfo.in_attributes(i).type() != core::pb::Attribute_Type_VEC3)
            {
                LogWarning(std::format("Vertex input {} must be vec3, found {}. Shader: {}",
                    shaderInfo.in_attributes(i).location(),
                    shaderInfo.in_attributes(i).type_name(),
                    shaderInfo.path()));
                return false;
            }
            break;
        }
        case 1:
        {
            if (shaderInfo.in_attributes(i).type() != core::pb::Attribute_Type_VEC2)
            {
                LogWarning(std::format("Vertex input {} must be vec2, found {}. Shader: {}",
                    shaderInfo.in_attributes(i).location(),
                    shaderInfo.in_attributes(i).type_name(),
                    shaderInfo.path()));
                return false;
            }
            break;
        }
        default: 
            break;
        }
    }
    return true;
    */
}
void ShaderEditor::AddResource(const Resource& resource)
{
    ShaderInfo shaderInfo{};
    shaderInfo.info.shader_stage = (core::GetShaderStageFromExtension(resource.extension));
    shaderInfo.compiledCorrectly = AnalyzeShader(resource.path, shaderInfo.info);
    shaderInfo.correctVertexInput = CheckVertexInput(shaderInfo.info);
    shaderInfo.filename = GetFilename(resource.path);
    shaderInfo.resourceId = resource.resourceId;
    shaderInfo.info.path = resource.path;
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
            shaderInfo.correctVertexInput = CheckVertexInput(shaderInfo.info);
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
    if(currentShaderInfo.info.shader_stage == internal::ShaderStage_VERTEX &&
        !currentShaderInfo.correctVertexInput)
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Incorrect vertex inputs!");

    }
    switch (currentShaderInfo.info.shader_stage)
    {
    case internal::ShaderStage_VERTEX:
    {
        ImGui::Text("Type: Vertex Shader");
        break;
    }
    case internal::ShaderStage_FRAGMENT:
    {
        ImGui::Text("Type: Fragment Shader");
        break;
    }
    case internal::ShaderStage_COMPUTE:
        ImGui::Text("Type: Compute Shader");
        break;

    default:
        break;
    }

    if(ImGui::BeginListBox("Uniforms"))
    {
        for(int i = 0; i < currentShaderInfo.info.uniform_buffers.size(); i++)
        {
            const auto& uniformInfo = currentShaderInfo.info.uniform_buffers[i];
            const auto text = std::format("Name: {} Type: {} Binding: {}", uniformInfo.name, uniformInfo.type_name, uniformInfo.binding);
            ImGui::Selectable(text.c_str(), false);
        }
        ImGui::EndListBox();
    }
    //TODO Show in attributes
    /*
    if(ImGui::BeginListBox("In Attributes"))
    {
        for(int i = 0; i < currentShaderInfo.info.in_attributes_size(); i++)
        {
            const auto& inAttributeInfo = currentShaderInfo.info.in_attributes(i);
            const auto text = std::format("Name: {} Type: {}", inAttributeInfo.name(), inAttributeInfo.type_name());
            ImGui::Selectable(text.c_str(), false);
        }
        ImGui::EndListBox();
    }
    */
    //TODO show out attributes
    /*
    if(ImGui::BeginListBox("Out Attributes"))
    {
        for(int i = 0; i < currentShaderInfo.info.out_attributes_size(); i++)
        {
            const auto& outAttributeInfo = currentShaderInfo.info.out_attributes(i);
            const auto text = std::format("Name: {} Type: {}", outAttributeInfo.name(), outAttributeInfo.type_name());
            ImGui::Selectable(text.c_str(), false);
        }
        ImGui::EndListBox();
    }
    */
    if (ImGui::BeginListBox("Structs"))
    {
        for (int i = 0; i < currentShaderInfo.info.types.size(); i++)
        {
            const auto& structInfo = currentShaderInfo.info.types[i];
            const auto text = std::format("Name: {}", structInfo.name);
            ImGui::Selectable(text.c_str(), false);
        }
        ImGui::EndListBox();
    }

    if (ImGui::BeginListBox("Buffers"))
    {
        for (int i = 0; i < currentShaderInfo.info.storage_buffers.size(); i++)
        {
            const auto& bufferInfo = currentShaderInfo.info.storage_buffers[i];
            const auto text = std::format("Name: {} Binding: {}", bufferInfo.name, bufferInfo.binding);
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
            std::string_view shaderPath{shaderInfo.info.path};
            if (core::FileExists(shaderPath))
            {
                const auto shaderContent = core::LoadFile(shaderPath);
                shaderText_ = reinterpret_cast<const char*>(shaderContent.data);
            }
            else
            {
                LogError(std::format("Could not load shader file: {} for central view", shaderPath.data()));
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
    core::WriteString(shaderInfos_[currentIndex_].info.path, shaderText_);
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
    resourceManager.RemoveResource(shaderInfos_[currentIndex_].info.path, true);
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

bool ShaderEditor::AnalyzeShader(std::string_view path, renderer::ShaderT& shaderInfo) const
{
    //TODO fill info about shader
    std::string result;
    json shaderJson;

    py::function analyzeShaderFunc = py::module_::import("scripts.shader_parser").attr("analyze_shader");
    try
    {
        result = static_cast<py::str>(analyzeShaderFunc(path.data()));
    }
    catch (py::error_already_set& e)
    {
        LogError(std::format("Analyze shader failed for file: {}\n{}", path, e.what()));
    }

    try
    {
        LogDebug(std::format("JSON content {}", result));
        shaderJson = json::parse(result);
        int returnCode = shaderJson["returncode"].get<int>();
        if (returnCode != 0)
        {
            LogDebug(shaderJson["stdout"].get<std::string>());
            LogError(shaderJson["stderr"].get<std::string>());
            return false;
        }
    }
    catch (json::exception& e)
    {
        LogError(std::format("Could not parse shader info of file: {} from script\n{}\n{}", path, e.what(), result));
    }

    const auto attributeResult = novus::GenerateShaderAttributeFromJson(std::string(path)+".json");
    shaderInfo.storage_buffers = std::move(attributeResult.storageBuffers);
    shaderInfo.samplers = std::move(attributeResult.shaderSamplers);
    shaderInfo.types = std::move(attributeResult.types);
    shaderInfo.uniform_buffers = std::move(attributeResult.uniformBuffers);
    return true;
}
}