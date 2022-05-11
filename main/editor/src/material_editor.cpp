#include "material_editor.h"
#include "editor.h"
#include "pipeline_editor.h"
#include "texture_editor.h"
#include "utils/log.h"
#include "command_editor.h"

#include "engine/filesystem.h"
#include <fmt/format.h>
#include <fstream>

namespace gpr5300
{
    

void MaterialEditor::DrawMainView()
{

}

void MaterialEditor::DrawInspector()
{
    if (currentIndex_ >= materialInfos_.size())
    {
        return;
    }

    const auto* editor = Editor::GetInstance();
    auto& currentMaterialInfo = materialInfos_[currentIndex_];

    const auto& resourceManager = editor->GetResourceManager();
    const auto* textureEditor = dynamic_cast<TextureEditor*>(editor->GetEditorSystem(EditorType::TEXTURE));
    const auto* pipelineEditor = dynamic_cast<PipelineEditor*>(editor->GetEditorSystem(EditorType::PIPELINE));
    const auto& pipelines = pipelineEditor->GetPipelines();
    const auto* pipelineInfo = pipelineEditor->GetPipeline(currentMaterialInfo.pipelineId);
    if(ImGui::BeginCombo("Pipeline", pipelineInfo?pipelineInfo->filename.c_str():"Empty pipeline"))
    {
        for(auto& pipeline : pipelines)
        {
            if(ImGui::Selectable(pipeline.filename.c_str(), pipeline.resourceId == currentMaterialInfo.pipelineId))
            {
                ReloadMaterialPipeline(pipeline, currentIndex_);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::Separator();
    ImGui::Text("Samplers");
    for(int i = 0; i < currentMaterialInfo.info.textures_size(); i++)
    {
        auto* materialTexture = currentMaterialInfo.info.mutable_textures(i);
        auto texturePath = materialTexture->texture_name();
        if (!texturePath.empty())
        {
            const auto textureId = resourceManager.FindResourceByPath(texturePath);
            if(textureId == INVALID_RESOURCE_ID)
            {
                materialTexture->clear_texture_name();
                texturePath = "";
            }
        }
        if(ImGui::BeginCombo(materialTexture->sampler_name().c_str(), 
            texturePath.empty()?"Empty texture":texturePath.c_str()))
        {
            for(const auto& texture : textureEditor->GetTextures())
            {
                if(ImGui::Selectable(texture.filename.c_str(), texture.info.path() == materialTexture->sampler_name()))
                {
                    materialTexture->set_texture_name(texture.info.path());
                }
            }

            ImGui::EndCombo();
        }        
    }
    ImGui::Separator();
    if(pipelineInfo != nullptr)
    {
        if (ImGui::BeginListBox("Uniforms"))
        {
            for(int i = 0; i < pipelineInfo->info.uniforms_size(); i++)
            {
                auto& uniformInfo = pipelineInfo->info.uniforms(i);
                const auto text = fmt::format("Name: {} Type: {}",
                                              uniformInfo.name(),
                                              uniformInfo.type_name());
                ImGui::Selectable(text.c_str(), false);
            }
            ImGui::EndListBox();
        }
        if (ImGui::BeginListBox("Vertex In Attributes"))
        {
            for(int i = 0; i < pipelineInfo->info.in_vertex_attributes_size(); i++)
            {
                auto& inVertexAttribute = pipelineInfo->info.in_vertex_attributes(i);
                const auto text = fmt::format("Name: {} Type: {}",
                                              inVertexAttribute.name(),
                                              inVertexAttribute.type_name());
                ImGui::Selectable(text.c_str(), false);
            }
            ImGui::EndListBox();
        }

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
    for(auto& materialInfo : materialInfos_)
    {
        std::ofstream fileOut(materialInfo.path, std::ios::binary);
        if (!materialInfo.info.SerializeToOstream(&fileOut))
        {
            LogWarning(fmt::format("Could not save material at: {}", materialInfo.path));
        }
        
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
    std::ifstream fileIn(resource.path, std::ios::binary);
    if (!materialInfo.info.ParsePartialFromIstream(&fileIn))
    {
        LogWarning(fmt::format("Could not open protobuf file: {}", resource.path));
        return;
    }
    materialInfos_.push_back(materialInfo);
}

void MaterialEditor::RemoveResource(const Resource &resource)
{
    const auto* editor = Editor::GetInstance();
    auto* commandEditor = dynamic_cast<CommandEditor*>(editor->GetEditorSystem(EditorType::COMMAND));
    for (auto& material : materialInfos_)
    {
        if (material.pipelineId == resource.resourceId)
        {
            material.info.clear_pipeline_path();
            material.pipelineId = INVALID_RESOURCE_ID;
            material.info.mutable_textures()->Clear();
        }
        
    }

    auto  it = std::ranges::find_if(materialInfos_, [&resource](const auto& material)
        {
            return resource.resourceId == material.resourceId;
        });
    if(it != materialInfos_.end())
    {
        materialInfos_.erase(it);
        commandEditor->RemoveResource(resource);
    }
}

void MaterialEditor::UpdateExistingResource(const Resource &resource)
{
    int i = 0;
    for(const auto& material : materialInfos_)
    {
        if (material.pipelineId == resource.resourceId)
        {
            const auto* editor = Editor::GetInstance();
            const auto* pipelineEditor = dynamic_cast<PipelineEditor*>(editor->GetEditorSystem(EditorType::PIPELINE));

            const auto* pipeline = pipelineEditor->GetPipeline(material.pipelineId);
            if (pipeline != nullptr)
            {
                ReloadMaterialPipeline(*pipeline, i);
            }
        }
        i++;
    }
}

MaterialInfo* MaterialEditor::GetMaterial(ResourceId resourceId)
{
    const auto it = std::ranges::find_if(materialInfos_, [resourceId](const auto& materialInfo)
    {
        return resourceId == materialInfo.resourceId;
    });
    if(it != materialInfos_.end())
    {
        return &*it;
    }
    return nullptr;
}

void MaterialEditor::ReloadId()
{
    const auto* editor = Editor::GetInstance();
    const auto& resourceManager = editor->GetResourceManager();
    const auto* pipelineEditor = dynamic_cast<PipelineEditor*>(editor->GetEditorSystem(EditorType::PIPELINE));
    int i = 0;
    for (auto& currentMaterialInfo : materialInfos_)
    {
        if (currentMaterialInfo.pipelineId == INVALID_RESOURCE_ID && !currentMaterialInfo.info.pipeline_path().empty())
        {
            currentMaterialInfo.pipelineId = resourceManager.FindResourceByPath(currentMaterialInfo.info.pipeline_path());
            const auto* pipeline = pipelineEditor->GetPipeline(currentMaterialInfo.pipelineId);
            if (pipeline != nullptr)
            {
                ReloadMaterialPipeline(*pipeline,i);
            }
        }
        i++;
    }
}

void MaterialEditor::Delete()
{
    if (currentIndex_ >= materialInfos_.size())
    {
        return;
    }
    auto* editor = Editor::GetInstance();
    auto& resourceManager = editor->GetResourceManager();
    resourceManager.RemoveResource(materialInfos_[currentIndex_].path);
}

std::span<const std::string_view> MaterialEditor::GetExtensions() const
{
    static constexpr std::array<std::string_view, 1> extensions = { ".mat" };
    return std::span{ extensions };
}

void MaterialEditor::ReloadMaterialPipeline(const PipelineInfo& pipelineInfo, int materialIndex)
{

    auto& currentMaterialInfo = materialInfos_[materialIndex];
    currentMaterialInfo.pipelineId = pipelineInfo.resourceId;
    currentMaterialInfo.info.set_pipeline_path(pipelineInfo.path);

    std::unordered_set<std::string> samplerNames;
    for (int i = 0; i < pipelineInfo.info.samplers_size(); i++)
    {
        const auto& samplerInfo = pipelineInfo.info.samplers(i);
        samplerNames.emplace(samplerInfo.name());
        
    }
    std::vector<pb::MaterialTexture> materialTextures;
    for(int i = 0; i < currentMaterialInfo.info.textures_size(); i++)
    {
        const auto& materialTexture = currentMaterialInfo.info.textures(i);
        if(samplerNames.contains(materialTexture.sampler_name()))
        {
            materialTextures.push_back(materialTexture);
        }
    }
    currentMaterialInfo.info.mutable_textures()->Clear();
    for(int i = 0; i < pipelineInfo.info.samplers_size(); i++)
    {
        auto* newMaterialTexture = currentMaterialInfo.info.add_textures();
        const auto& sampler = pipelineInfo.info.samplers(i);
        const auto it = std::ranges::find_if(materialTextures, [&sampler](const auto& matText)
            {
                return sampler.name() == matText.sampler_name();
            });
        if (it != materialTextures.end())
        {
            *newMaterialTexture = *it;
            if (newMaterialTexture->texture_type() == pb::NONE && sampler.type() != pb::NONE)
            {
                newMaterialTexture->set_texture_type(sampler.type());
            }
        }
        else
        {
            newMaterialTexture->set_sampler_name(sampler.name());
            newMaterialTexture->set_texture_type(sampler.type());
        }
    }
}
}
