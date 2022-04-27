#include "pipeline_editor.h"
#include "editor.h"
#include "shader_editor.h"
#include "engine/filesystem.h"
#include "utils/log.h"
#include <fmt/format.h>
#include <imgui_stdlib.h>
#include <fstream>

namespace gpr5300
{

bool PipelineEditor::CheckExtensions(std::string_view extension)
{
    return extension == ".pipe";
}
void PipelineEditor::DrawMainView()
{

}
void PipelineEditor::DrawInspector()
{
    if (currentIndex_ >= pipelineInfos_.size())
    {
        return;
    }
    auto* editor = Editor::GetInstance();
    const auto& resourceManager = editor->GetResourceManager();
    auto& currentPipelineInfo = pipelineInfos_[currentIndex_];

    //Pipeline type
    if(currentPipelineInfo.info.type() != pb::Pipeline_Type_RASTERIZE &&
        currentPipelineInfo.info.type() != pb::Pipeline_Type_COMPUTE)
    {
        currentPipelineInfo.info.set_type(pb::Pipeline_Type_RASTERIZE);
    }
    int index = currentPipelineInfo.info.type();
    const char* pipelineTypeText[] = {
        "Rasterizer",
        "Compute"
    };
    if(ImGui::Combo("Combo", &index, pipelineTypeText, IM_ARRAYSIZE(pipelineTypeText)))
    {
        currentPipelineInfo.info.set_type(static_cast<pb::Pipeline_Type>(index));
    }

    //Rasterizer pipeline
    if (currentPipelineInfo.info.type() == pb::Pipeline_Type_RASTERIZE)
    {
        auto* shaderEditor = dynamic_cast<ShaderEditor*>(editor->GetEditorSystem(EditorType::SHADER));
        const auto& shaders = shaderEditor->GetShaders();
        const auto* vertexShader = shaderEditor->GetShader(currentPipelineInfo.vertexShaderId);
        if (ImGui::BeginCombo("Vertex Shader", vertexShader ? vertexShader->filename.data() : "No vertex shader"))
        {
            for (auto& shader : shaders)
            {
                if (shader.info.type() != pb::Shader_Type_VERTEX)
                {
                    continue;
                }
                if (ImGui::Selectable(shader.filename.c_str(), shader.resourceId == currentPipelineInfo.vertexShaderId))
                {
                    currentPipelineInfo.vertexShaderId = shader.resourceId;
                    currentPipelineInfo.info.set_vertex_shader_path(shader.info.path());
                }
            }
            ImGui::EndCombo();
        }

        const auto* fragmentShader = shaderEditor->GetShader(currentPipelineInfo.fragmentShaderId);
        if (ImGui::BeginCombo("Fragment Shader", fragmentShader ? fragmentShader->filename.data() : "No fragment shader"))
        {
            for (auto& shader : shaders)
            {
                if (shader.info.type() != pb::Shader_Type_FRAGMENT)
                {
                    continue;
                }
                if (ImGui::Selectable(shader.filename.c_str(), shader.resourceId == currentPipelineInfo.fragmentShaderId))
                {
                    currentPipelineInfo.fragmentShaderId = shader.resourceId;
                    currentPipelineInfo.info.set_fragment_shader_path(shader.info.path());
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginListBox("Uniforms"))
        {
            if (vertexShader != nullptr)
            {
                for (int i = 0; i < vertexShader->info.uniforms_size(); i++)
                {
                    const auto& uniformInfo = vertexShader->info.uniforms(i);
                    const auto text = fmt::format("Name: {} Type: {}", uniformInfo.name(), uniformInfo.type_name());
                    ImGui::Selectable(text.c_str(), false);
                }
            }
            if (fragmentShader != nullptr)
            {
                for (int i = 0; i < fragmentShader->info.uniforms_size(); i++)
                {
                    const auto& uniformInfo = fragmentShader->info.uniforms(i);
                    const auto text = fmt::format("Name: {} Type: {}", uniformInfo.name(), uniformInfo.type_name());
                    ImGui::Selectable(text.c_str(), false);
                }
            }
            ImGui::EndListBox();
        }

        if (ImGui::BeginListBox("In Attributes"))
        {
            if (vertexShader != nullptr)
            {
                for (int i = 0; i < vertexShader->info.in_attributes_size(); i++)
                {
                    const auto& inAttributeInfo = vertexShader->info.in_attributes(i);
                    const auto text = fmt::format("Name: {} Type: {}", inAttributeInfo.name(), inAttributeInfo.type_name());
                    ImGui::Selectable(text.c_str(), false);
                }
            }
            ImGui::EndListBox();
        }
    }
}
bool PipelineEditor::DrawContentList(bool unfocus)
{
    bool wasFocused = false;
    if (unfocus)
        currentIndex_ = pipelineInfos_.size();
    for (std::size_t i = 0; i < pipelineInfos_.size(); i++)
    {
        const auto& shaderInfo = pipelineInfos_[i];
        if (ImGui::Selectable(shaderInfo.filename.data(),  currentIndex_ == i))
        {
            currentIndex_ = i;
            wasFocused = true;
        }
    }
    return wasFocused;
}
std::string_view PipelineEditor::GetSubFolder()
{
    return "pipelines/";
}
EditorType PipelineEditor::GetEditorType()
{
    return EditorType::PIPELINE;
}
void PipelineEditor::AddResource(const Resource& resource)
{
    PipelineInfo pipelineInfo{};
    pipelineInfo.filename = GetFilename(resource.path);
    pipelineInfo.resourceId = resource.resourceId;
    pipelineInfo.path = resource.path;

    const auto& fileSystem = FilesystemLocator::get();

    if (!fileSystem.IsRegularFile(resource.path))
    {
        LogWarning(fmt::format("Could not find pipeline file: {}", resource.path));
        return;
    }
    std::ifstream fileIn (resource.path, std::ios::binary);
    if (!pipelineInfo.info.ParseFromIstream(&fileIn))
    {
        LogWarning(fmt::format("Could not open protobuf file: {}", resource.path));
        return;
    }
    pipelineInfos_.push_back(pipelineInfo);
}
void PipelineEditor::RemoveResource(const Resource& resource)
{

}
void PipelineEditor::UpdateExistingResource(const Resource& resource)
{

}

void PipelineEditor::Save()
{
    for(auto& pipelineInfo : pipelineInfos_)
    {
        std::ofstream fileOut(pipelineInfo.path, std::ios::binary);
        if (!pipelineInfo.info.SerializeToOstream(&fileOut))
        {
            LogWarning(fmt::format("Could not save pipeline at: {}", pipelineInfo.path));
        }
        
    }
}

const PipelineInfo *PipelineEditor::GetPipeline(ResourceId resourceId) const {
    const auto it = std::ranges::find_if(pipelineInfos_, [resourceId](const auto& pipeline)
    {
        return resourceId == pipeline.resourceId;
    });
    if(it != pipelineInfos_.end())
    {
        return &*it;
    }
    return nullptr;
}
void PipelineEditor::ReloadId()
{
    auto* editor = Editor::GetInstance();
    const auto& resourceManager = editor->GetResourceManager();
    for (auto& currentPipelineInfo : pipelineInfos_)
    {
        if (currentPipelineInfo.info.type() == pb::Pipeline_Type_RASTERIZE)
        {
            if (currentPipelineInfo.vertexShaderId == INVALID_RESOURCE_ID && !currentPipelineInfo.info.vertex_shader_path().empty())
            {
                currentPipelineInfo.vertexShaderId = resourceManager.FindResourceByPath(currentPipelineInfo.info.vertex_shader_path());
            }

            if (currentPipelineInfo.fragmentShaderId == INVALID_RESOURCE_ID && !currentPipelineInfo.info.fragment_shader_path().empty())
            {
                currentPipelineInfo.fragmentShaderId = resourceManager.FindResourceByPath(currentPipelineInfo.info.fragment_shader_path());
            }
        }
    }
}
}
