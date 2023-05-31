#include "material_editor.h"
#include "editor.h"
#include "pipeline_editor.h"
#include "texture_editor.h"
#include "utils/log.h"
#include "command_editor.h"
#include "framebuffer_editor.h"

#include "engine/filesystem.h"
#include <fmt/format.h>
#include <fstream>

#include "imnodes.h"

namespace editor
{
    
    

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
    auto* framebufferEditor = dynamic_cast<FramebufferEditor*>(editor->GetEditorSystem(EditorType::FRAMEBUFFER));
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
        const auto& fbAttachment = materialTexture->material_texture().attachment_name();
        if (!texturePath.empty())
        {

            const auto textureId = resourceManager.FindResourceByPath(texturePath);
            if(textureId == INVALID_RESOURCE_ID)
            {
                materialTexture->clear_texture_name();
                texturePath = "";
            }
        }
        if(ImGui::BeginCombo(materialTexture->material_texture().sampler_name().c_str(), 
            texturePath.empty() && fbAttachment.empty() ? "Empty texture" : texturePath.empty()?fbAttachment.data():texturePath.data()))
        {
            for(const auto& texture : textureEditor->GetTextures())
            {
                if(ImGui::Selectable(texture.filename.c_str(), texture.info.path() == materialTexture->material_texture().sampler_name()))
                {
                    materialTexture->mutable_material_texture()->clear_attachment_name();
                    materialTexture->set_texture_name(texture.info.path());
                }
            }
            const auto& framebuffers = framebufferEditor->GetFramebuffers();
            if (!framebuffers.empty())
            {
                ImGui::Separator();
                ImGui::Text("Framebuffer Attachment");
                ImGui::Separator();

                for (auto& framebuffer : framebuffers)
                {
                    const auto& framebufferName = framebuffer.info.name();

                    for (auto& colorAttachment : framebuffer.info.color_attachments())
                    {
                        if (colorAttachment.rbo())
                            continue;
                        const auto& colorAttachmentName = colorAttachment.name();
                        const auto attachmentUniqueName = fmt::format("{}_{}", framebufferName, colorAttachmentName);
                        if (ImGui::Selectable(attachmentUniqueName.data(), 
                            colorAttachmentName == materialTexture->material_texture().sampler_name() &&
                            framebufferName == materialTexture->material_texture().framebuffer_name()))
                        {
                            materialTexture->clear_texture_name();
                            materialTexture->mutable_material_texture()->set_attachment_name(colorAttachmentName);
                            materialTexture->mutable_material_texture()->set_framebuffer_name(framebufferName);
                        }
                    }
                    if(framebuffer.info.has_depth_stencil_attachment() && !framebuffer.info.depth_stencil_attachment().rbo())
                    {
                        const auto& depthAttachmentName = framebuffer.info.depth_stencil_attachment().name();
                        const auto attachmentUniqueName = fmt::format("{}_{}", framebufferName, depthAttachmentName);
                        if(ImGui::Selectable(attachmentUniqueName.data(), 
                            materialTexture->material_texture().attachment_name() == depthAttachmentName &&
                            materialTexture->material_texture().framebuffer_name() == framebufferName))
                        {
                            materialTexture->clear_texture_name();
                            materialTexture->mutable_material_texture()->set_attachment_name(depthAttachmentName);
                            materialTexture->mutable_material_texture()->set_framebuffer_name(framebufferName);
                        }
                    }
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
            for(int i = 0; i < pipelineInfo->info.pipeline().uniforms_size(); i++)
            {
                auto& uniformInfo = pipelineInfo->info.pipeline().uniforms(i);
                const auto text = fmt::format("Name: {} Type: {}",
                                              uniformInfo.name(),
                                              uniformInfo.type_name());
                ImGui::Selectable(text.c_str(), false);
            }
            ImGui::EndListBox();
        }
        if (ImGui::BeginListBox("Vertex In Attributes"))
        {
            for(int i = 0; i < pipelineInfo->info.pipeline().in_vertex_attributes_size(); i++)
            {
                auto& inVertexAttribute = pipelineInfo->info.pipeline().in_vertex_attributes(i);
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

void MaterialEditor::DrawCenterView()
{
    if (currentIndex_ >= materialInfos_.size())
        return;
    const auto* editor = Editor::GetInstance();
    const auto* pipelineEditor = dynamic_cast<PipelineEditor*>(editor->GetEditorSystem(EditorType::PIPELINE));
    const auto& resourceManager = editor->GetResourceManager();

    const auto& currentMaterial = materialInfos_[currentIndex_];

    constexpr auto textureBaseIndex = 100;
    constexpr auto uniformsBaseIndex = 200;
    constexpr int samplerBaseIndex = 300;

    std::vector<std::pair<int, int>> links;
    links.reserve(currentMaterial.info.textures_size() + currentMaterial.info.material().uniforms_size());

    ImNodes::BeginNodeEditor();

    ImNodes::BeginNode(0);
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Textures");
    ImNodes::EndNodeTitleBar();
    for(int i = 0; i < currentMaterial.info.textures_size(); i++)
    {
        const auto& materialTexture = currentMaterial.info.textures(i);
        ImNodes::BeginOutputAttribute(textureBaseIndex+i);
        auto textureName = GetFilename(materialTexture.texture_name());
        const auto textureType = aiTextureTypeToString(static_cast<aiTextureType>(materialTexture.texture_type()));
        ImGui::Text("%s %s", textureName.c_str(), textureType);
        ImNodes::EndOutputAttribute();
    }
    ImNodes::EndNode();

    ImNodes::SetNodeGridSpacePos(0, { 50,50 });

    ImNodes::BeginNode(1);
    
    ImNodes::BeginNodeTitleBar();
    
    ImGui::TextUnformatted(currentMaterial.pipelineId != INVALID_RESOURCE_ID ?"Uniforms" : "Missing Pipeline Id");
    ImNodes::EndNodeTitleBar();
    if (currentMaterial.pipelineId != INVALID_RESOURCE_ID)
    {
        const auto* pipeline = pipelineEditor->GetPipeline(currentMaterial.pipelineId);
        for (int i = 0; i < pipeline->info.pipeline().uniforms_size(); i++)
        {
            const auto& uniform = pipeline->info.pipeline().uniforms(i);
            if (uniform.type() == core::pb::Attribute_Type_SAMPLER2D || uniform.type() == core::pb::Attribute_Type_SAMPLERCUBE)
                continue;
            ImNodes::BeginOutputAttribute(uniformsBaseIndex + i);
            ImGui::Text("%s %s", uniform.type_name().c_str(), uniform.name().c_str());
            ImNodes::EndOutputAttribute();
            links.emplace_back(0, uniformsBaseIndex + i);

        }
    }
    ImNodes::EndNode();
    ImNodes::SetNodeGridSpacePos(1, { 50,250 });

    ImNodes::BeginNode(2);
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Material");
    ImNodes::EndNodeTitleBar();
    ImNodes::BeginInputAttribute(0);
    ImGui::TextUnformatted("Uniforms");
    ImNodes::EndInputAttribute();
    if (currentMaterial.pipelineId != INVALID_RESOURCE_ID)
    {
        const auto* pipeline = pipelineEditor->GetPipeline(currentMaterial.pipelineId);

        ImGui::TextUnformatted("Samplers:");
        for (int i = 0; i < pipeline->info.pipeline().uniforms_size(); i++)
        {
            const auto& uniform = pipeline->info.pipeline().uniforms(i);

            if (uniform.type() != core::pb::Attribute_Type_SAMPLER2D &&
                uniform.type() != core::pb::Attribute_Type_SAMPLERCUBE)
                continue;
            ImNodes::BeginInputAttribute(samplerBaseIndex + i);
            auto it = std::ranges::find_if(currentMaterial.info.textures(), [&uniform](const auto& materialTexture)
                {
                    return materialTexture.material_texture().sampler_name() == uniform.name();
                });
            if (it == currentMaterial.info.textures().cend())
            {
                //TODO add combo menu to select texture
                ImGui::TextColored({ 1,0,0,1 }, "Missing texture");
            }
            else
            {
                ImGui::Text("%s", uniform.name().c_str());
                links.emplace_back(
                    samplerBaseIndex + i, 
                    textureBaseIndex + static_cast<int>(std::distance(currentMaterial.info.textures().cbegin(), it)));
            }
            ImNodes::EndInputAttribute();

        }
    }
    ImNodes::EndNode();

    ImNodes::SetNodeGridSpacePos(2, { 300,150 });

    for (std::size_t i = 0; i < links.size(); i++)
    {
        ImNodes::Link(i, links[i].first, links[i].second);
    }
    ImNodes::EndNodeEditor();
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
    const auto& fileSystem = core::FilesystemLocator::get();

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
    if(materialInfo.info.material().name().empty())
    {
        materialInfo.info.mutable_material()->set_name(GetFilename(resource.path, false));
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
    resourceManager.RemoveResource(materialInfos_[currentIndex_].path, true);
}

std::span<const std::string_view> MaterialEditor::GetExtensions() const
{
    static constexpr std::array<std::string_view, 1> extensions = { ".mat" };
    return std::span{ extensions };
}

void MaterialEditor::Clear()
{
    materialInfos_.clear();
    currentIndex_ = -1;
}

void MaterialEditor::ReloadMaterialPipeline(const PipelineInfo& pipelineInfo, int materialIndex)
{

    auto& currentMaterialInfo = materialInfos_[materialIndex];
    currentMaterialInfo.pipelineId = pipelineInfo.resourceId;
    currentMaterialInfo.info.set_pipeline_path(pipelineInfo.path);

    std::unordered_set<std::string> samplerNames;
    for (int i = 0; i < pipelineInfo.info.pipeline().samplers_size(); i++)
    {
        const auto& samplerInfo = pipelineInfo.info.pipeline().samplers(i);
        samplerNames.emplace(samplerInfo.name());
        
    }
    std::vector<editor::pb::EditorMaterialTexture> materialTextures;
    for(int i = 0; i < currentMaterialInfo.info.textures_size(); i++)
    {
        const auto& materialTexture = currentMaterialInfo.info.textures(i);
        if(samplerNames.contains(materialTexture.material_texture().sampler_name()))
        {
            materialTextures.push_back(materialTexture);
        }
    }
    currentMaterialInfo.info.mutable_textures()->Clear();
    for(int i = 0; i < pipelineInfo.info.pipeline().samplers_size(); i++)
    {
        auto* newMaterialTexture = currentMaterialInfo.info.add_textures();
        const auto& sampler = pipelineInfo.info.pipeline().samplers(i);
        const auto it = std::ranges::find_if(materialTextures, [&sampler](const auto& matText)
            {
                return sampler.name() == matText.material_texture().sampler_name();
            });
        if (it != materialTextures.end())
        {
            *newMaterialTexture = *it;
            if (newMaterialTexture->texture_type() == core::pb::NONE && sampler.type() != core::pb::NONE)
            {
                newMaterialTexture->set_texture_type(sampler.type());
            }
        }
        else
        {
            newMaterialTexture->mutable_material_texture()->set_sampler_name(sampler.name());
            newMaterialTexture->set_texture_type(sampler.type());
        }
    }
}
}
