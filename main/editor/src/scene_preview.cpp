#include "scene_preview.h"

namespace gpr5300
{

bool SceneEditor::CheckExtensions(std::string_view extension) {
    return false;
}

void SceneEditor::DrawMainView() {

}

void SceneEditor::DrawInspector() {

}

bool SceneEditor::DrawContentList(bool unfocus) {
    return false;
}

std::string_view SceneEditor::GetSubFolder() {
    return "scene/";
}

EditorType SceneEditor::GetEditorType() {
    return EditorType::PIPELINE;
}

void SceneEditor::Save() {

}

void SceneEditor::AddResource(const Resource &resource) {

}

void SceneEditor::RemoveResource(const Resource &resource) {

}

void SceneEditor::UpdateExistingResource(const Resource &resource) {

}
}