#include "render_pass_editor.h"
#include "proto/renderer.pb.h"
#include "resource.h"
#include "editor_system.h"

namespace gpr5300
{


bool RenderPassEditor::CheckExtensions(std::string_view extension)
{
    return false;
}

void RenderPassEditor::DrawMainView() {

}

void RenderPassEditor::DrawInspector()
{

}

bool RenderPassEditor::DrawContentList(bool unfocus) {
    return false;
}

std::string_view RenderPassEditor::GetSubFolder() {
    return "render_passes/";
}

EditorType RenderPassEditor::GetEditorType() {
    return EditorType::PIPELINE;
}

void RenderPassEditor::Save() {

}

void RenderPassEditor::AddResource(const Resource &resource)
{

}

void RenderPassEditor::RemoveResource(const Resource &resource)
{

}

void RenderPassEditor::UpdateResource(const Resource &resource)
{

}
}