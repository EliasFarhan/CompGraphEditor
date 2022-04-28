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
        if(currentPipelineInfo.vertexShaderId == INVALID_RESOURCE_ID || 
            currentPipelineInfo.fragmentShaderId == INVALID_RESOURCE_ID)
        {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Pipeline is not completed (require at Vertex and Fragment shader)");
        }
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
                    ReloadPipeline(currentIndex_);
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
                    ReloadPipeline(currentIndex_);
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginListBox("Uniforms"))
        {
            for (int i = 0; i < currentPipelineInfo.info.uniforms_size(); i++)
            {
                const auto& uniformInfo = currentPipelineInfo.info.uniforms(i);
                const auto text = fmt::format("Name: {} Type: {}", uniformInfo.name(), uniformInfo.type_name());
                ImGui::Selectable(text.c_str(), false);
            }
            
            ImGui::EndListBox();
        }

        if (ImGui::BeginListBox("In Attributes"))
        {
            for (int i = 0; i < currentPipelineInfo.info.in_vertex_attributes_size(); i++)
            {
                const auto& inAttributeInfo = currentPipelineInfo.info.in_vertex_attributes(i);
                const auto text = fmt::format("Name: {} Type: {}", inAttributeInfo.name(), inAttributeInfo.type_name());
                ImGui::Selectable(text.c_str(), false);
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
    int i = 0; 
    for(auto& pipelineInfo : pipelineInfos_)
    {
        bool modified = false;
        if(pipelineInfo.vertexShaderId == resource.resourceId)
        {
            pipelineInfo.vertexShaderId = INVALID_RESOURCE_ID;
            pipelineInfo.info.clear_vertex_shader_path();
            modified = true;
        }
        if (pipelineInfo.fragmentShaderId == resource.resourceId)
        {
            pipelineInfo.fragmentShaderId = INVALID_RESOURCE_ID;
            pipelineInfo.info.clear_fragment_shader_path();
            modified = true;
        }
        if(modified)
        {
            ReloadPipeline(i);
        }
        i++;
    }

    const auto it = std::ranges::find_if(pipelineInfos_, [&resource](const auto& pipelineInfo)
        {
            return resource.resourceId == pipelineInfo.resourceId;
        });
    if(it != pipelineInfos_.end())
    {
        pipelineInfos_.erase(it);
        const auto* editor = Editor::GetInstance();
        auto* materialEditor = editor->GetEditorSystem(EditorType::MATERIAL);
        materialEditor->RemoveResource(resource);
    }
}
void PipelineEditor::UpdateExistingResource(const Resource& resource)
{
    //update pipeline if shader modifies it
    int i = 0;
    for (auto& pipelineInfo : pipelineInfos_)
    {
        bool modified = false;
        if (pipelineInfo.vertexShaderId == resource.resourceId)
        {
            modified = true;
        }
        if (pipelineInfo.fragmentShaderId == resource.resourceId)
        {
            modified = true;
        }
        if (modified)
        {
            ReloadPipeline(i);
            const auto* editor = Editor::GetInstance();
            const auto& resourceManager = editor->GetResourceManager();
            auto* materialEditor = editor->GetEditorSystem(EditorType::MATERIAL);

            const auto* pipelineResource = resourceManager.GetResource(pipelineInfo.resourceId);
            if (pipelineResource != nullptr)
            {
                materialEditor->UpdateExistingResource(*pipelineResource);
            }
        }
        i++;
    }
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
    
    for (std::size_t i = 0; i < pipelineInfos_.size();i++)
    {
        ReloadPipeline(i);
    }
}

void PipelineEditor::ReloadPipeline(int index)
{
    if(index >= pipelineInfos_.size())
    {
        return;
    }
    auto* editor = Editor::GetInstance();
    const auto& resourceManager = editor->GetResourceManager();
    const auto* shaderEditor = dynamic_cast<ShaderEditor*>(editor->GetEditorSystem(EditorType::SHADER));

    auto& pipelineInfo = pipelineInfos_[index];
    if (pipelineInfo.info.type() == pb::Pipeline_Type_RASTERIZE)
    {
        if (pipelineInfo.vertexShaderId == INVALID_RESOURCE_ID && !pipelineInfo.info.vertex_shader_path().empty())
        {
            pipelineInfo.vertexShaderId = resourceManager.FindResourceByPath(pipelineInfo.info.vertex_shader_path());
        }

        if (pipelineInfo.fragmentShaderId == INVALID_RESOURCE_ID && !pipelineInfo.info.fragment_shader_path().empty())
        {
            pipelineInfo.fragmentShaderId = resourceManager.FindResourceByPath(pipelineInfo.info.fragment_shader_path());
        }
    }
    pipelineInfo.info.mutable_uniforms()->Clear();
    pipelineInfo.info.mutable_in_vertex_attributes()->Clear();
    //load uniforms and input attributes to pipeline
    if(pipelineInfo.vertexShaderId != INVALID_RESOURCE_ID)
    {
        const auto* vertexShader = shaderEditor->GetShader(pipelineInfo.vertexShaderId);
        for(int i = 0; i < vertexShader->info.in_attributes_size(); i++)
        {
            *pipelineInfo.info.add_in_vertex_attributes() = vertexShader->info.in_attributes(i);
        }
        for(int i = 0; i < vertexShader->info.uniforms_size(); i++)
        {
            *pipelineInfo.info.add_uniforms() = vertexShader->info.uniforms(i);
        }
    }
    if(pipelineInfo.fragmentShaderId != INVALID_RESOURCE_ID)
    {
        const auto* fragmentShader = shaderEditor->GetShader(pipelineInfo.fragmentShaderId);
        for (int i = 0; i < fragmentShader->info.uniforms_size(); i++)
        {
            *pipelineInfo.info.add_uniforms() = fragmentShader->info.uniforms(i);
        }
    }
}
}
