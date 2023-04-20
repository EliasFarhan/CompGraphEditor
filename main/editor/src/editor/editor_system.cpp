#include "editor_system.h"
#include "resource_manager.h"
#include "editor.h"

#include <fmt/format.h>

#include "scene_editor.h"

namespace editor
{

void EditorSystem::ImportResource(std::string_view path)
{
    const auto& sceneName = GetSceneEditor()->GetCurrentSceneInfo()->info.name();
    const auto dstPath = fmt::format("{}{}/{}{}",
        ResourceManager::dataFolder,
        sceneName,
        GetSubFolder(),
        GetFilename(path));
    if (CopyFileFromTo(path, dstPath))
    {
        auto* editor = Editor::GetInstance();
        auto& resourceManager = editor->GetResourceManager();
        resourceManager.AddResource(dstPath);
    }
}
}
