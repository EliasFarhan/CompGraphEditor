#include "texture_editor.h"

namespace gpr5300
{

bool TextureEditor::CheckExtensions(std::string_view extension)
{
    return false;
}

void TextureEditor::DrawMainView() {

}

void TextureEditor::DrawInspector() {

}

bool TextureEditor::DrawContentList(bool unfocus)
{
    return false;
}

std::string_view TextureEditor::GetSubFolder() {
    return "textures/";
}

EditorType TextureEditor::GetEditorType() {
    return EditorType::PIPELINE;
}

void TextureEditor::Save()
{

}

void TextureEditor::AddResource(const Resource &resource) {

}

void TextureEditor::RemoveResource(const Resource &resource) {

}

void TextureEditor::UpdateExistingResource(const Resource &resource) {

}
}