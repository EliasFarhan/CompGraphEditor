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
    if (currentIndex_ >= materialInfos_.size())
    {
        return;
    }

    auto* editor = Editor::GetInstance();
    const auto& resourceManager = editor->GetResourceManager();
    auto& currentMaterialInfo = materialInfos_[currentIndex_];

    if(currentMaterialInfo.pipelineId == INVALID_RESOURCE_ID && !currentMaterialInfo.info.pipeline_path().empty())
    {
        currentMaterialInfo.pipelineId = resourceManager.FindResourceByPath(currentMaterialInfo.info.pipeline_path());
    }

    auto* pipelineEditor = dynamic_cast<PipelineEditor*>(editor->GetEditorSystem(EditorType::PIPELINE));
    const auto& pipelines = pipelineEditor->GetPipelines();
    const auto* pipelineInfo = pipelineEditor->GetPipeline(currentMaterialInfo.pipelineId);
    if(ImGui::BeginCombo("Pipeline", pipelineInfo?pipelineInfo->filename.c_str():"Empty pipeline"))
    {
        for(auto& pipeline : pipelines)
        {
            if(ImGui::Selectable(pipeline.filename.c_str(), pipeline.resourceId == currentMaterialInfo.pipelineId))
            {
                currentMaterialInfo.pipelineId = pipeline.resourceId;
                currentMaterialInfo.info.set_pipeline_path(pipeline.path);
            }
        }
        ImGui::EndCombo();
    }

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

        //TODO add and select textures
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