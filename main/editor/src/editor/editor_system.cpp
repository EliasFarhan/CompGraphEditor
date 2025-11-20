#include "editor_system.h"
#include "resource_manager.h"
#include "editor.h"

#include <format>

#include "scene_editor.h"

namespace novus::editor
{

void EditorSystem::ImportResource(std::string_view path)
{
    const auto& sceneName = GetSceneEditor()->GetCurrentSceneInfo()->name;
    const std::string dstPath{std::format("{}{}/{}{}",
        ResourceManager::dataFolder.data(),
        sceneName,
        GetSubFolder(),
        GetFilename(path))};
    if (CopyFileFromTo(path, dstPath))
    {
        auto* editor = Editor::GetInstance();
        auto& resourceManager = editor->GetResourceManager();
        resourceManager.AddResource(dstPath);
        GetSceneEditor()->AddResource(*resourceManager.GetResource(resourceManager.FindResourceByPath(dstPath)));
    }
}
}
