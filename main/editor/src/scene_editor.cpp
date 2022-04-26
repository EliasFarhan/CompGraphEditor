#include "scene_editor.h"
#include "engine/filesystem.h"
#include "utils/log.h"
#include "render_pass_editor.h"

#include <imgui.h>
#include <fmt/format.h>
#include <fstream>

#include "editor.h"

namespace gpr5300
{
void SceneEditor::AddResource(const Resource& resource)
{
    SceneInfo commandInfo{};
    commandInfo.resourceId = resource.resourceId;
    commandInfo.filename = GetFilename(resource.path);

    const auto extension = GetFileExtension(resource.path);
    if (extension == ".scene")
    {
        const auto& fileSystem = FilesystemLocator::get();
        if (!fileSystem.IsRegularFile(resource.path))
        {
            LogWarning(fmt::format("Could not find scene file: {}", resource.path));
            return;
        }
        std::ifstream fileIn(resource.path, std::ios::binary);
        if (!commandInfo.info.ParseFromIstream(&fileIn))
        {
            LogWarning(fmt::format("Could not open protobuf file: {}", resource.path));
            return;
        }
    }
    commandInfo.path = resource.path;
    sceneInfos_.push_back(commandInfo);
}

void SceneEditor::RemoveResource(const Resource& resource)
{
}

void SceneEditor::UpdateExistingResource(const Resource& resource)
{
}

bool SceneEditor::CheckExtensions(std::string_view extension)
{
    return extension == ".scene";
}

void SceneEditor::DrawMainView()
{
}

void SceneEditor::DrawInspector()
{
    if (currentIndex_ >= sceneInfos_.size())
    {
        return;
    }
    auto* editor = Editor::GetInstance();
    const auto& resourceManager = editor->GetResourceManager();
    auto* renderPassEditor = dynamic_cast<RenderPassEditor*>(editor->GetEditorSystem(EditorType::RENDER_PASS));
    auto& currentScene = sceneInfos_[currentIndex_];

    if(currentScene.renderPassId == INVALID_RESOURCE_ID && !currentScene.info.render_pass_path().empty())
    {
        currentScene.renderPassId = resourceManager.FindResourceByPath(currentScene.info.render_pass_path());
    }

    const auto& renderPasses = renderPassEditor->GetRenderPasses();
    const auto* renderPassInfo = renderPassEditor->GetRenderPass(currentScene.renderPassId);
    if(ImGui::BeginCombo("Render Pass", renderPassInfo?renderPassInfo->filename.c_str():"Empty Render Pass"))
    {
        for(auto& renderPass : renderPasses)
        {
            if(ImGui::Selectable(renderPass.filename.c_str(), renderPass.resourceId == currentScene.renderPassId))
            {
                currentScene.renderPassId = renderPass.resourceId;
                currentScene.info.set_render_pass_path(renderPass.path);
            }
        }
        ImGui::EndCombo();
    }
}

bool SceneEditor::DrawContentList(bool unfocus)
{
    bool wasFocused = false;
    if (unfocus)
        currentIndex_ = sceneInfos_.size();
    for (std::size_t i = 0; i < sceneInfos_.size(); i++)
    {
        const auto& sceneInfo = sceneInfos_[i];
        if (ImGui::Selectable(sceneInfo.filename.data(), currentIndex_ == i))
        {
            currentIndex_ = i;
            wasFocused = true;
        }
    }
    return wasFocused;
}

std::string_view SceneEditor::GetSubFolder()
{
    return "scenes/";
}

EditorType SceneEditor::GetEditorType()
{
    return EditorType::SCENE;
}

void SceneEditor::Save()
{
    for (auto& sceneInfo : sceneInfos_)
    {
        std::ofstream fileOut(sceneInfo.path, std::ios::binary);
        if (!sceneInfo.info.SerializeToOstream(&fileOut))
        {
            LogWarning(fmt::format("Could not save scene at: {}", sceneInfo.path));
        }

    }
}
}
