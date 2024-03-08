#include "editor_system.h"
#include "resource_manager.h"
#include "editor.h"

#include <fmt/format.h>

#include "scene_editor.h"

namespace editor
{

void EditorSystem::ImportResource(const core::Path &path)
{
    const auto& sceneName = GetSceneEditor()->GetCurrentSceneInfo()->info.name();
    const core::Path dstPath{fmt::format("{}{}/{}{}",
        ResourceManager::dataFolder,
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
