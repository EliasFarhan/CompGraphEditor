#include "pipeline_editor.h"

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

}
bool PipelineEditor::DrawContentList(bool unfocus)
{
    return false;
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

}
void PipelineEditor::RemoveResource(const Resource& resource)
{

}
void PipelineEditor::UpdateResource(const Resource& resource)
{

}
}