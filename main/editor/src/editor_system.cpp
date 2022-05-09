#include "editor_system.h"
#include "resource_manager.h"
#include "editor.h"

#include <fmt/format.h>

namespace gpr5300
{
void EditorSystem::ImportResource(std::string_view path)
{
    const auto dstPath = fmt::format("{}{}{}",
        ResourceManager::dataFolder,
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
