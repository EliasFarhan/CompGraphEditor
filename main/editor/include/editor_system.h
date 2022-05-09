#pragma once
#include <string_view>
#include "resource.h"
#include <span>

namespace gpr5300
{

enum class EditorType
{
    SHADER,
    PIPELINE,
    TEXTURE,
    MESH,
    MATERIAL,
    SCENE,
    RENDER_PASS,
    COMMAND,
    SCRIPT,
    MODEL,
    LENGTH
};

class EditorSystem : public ResourceChangeInterface
{
public:
    virtual ~EditorSystem() = default;

    [[nodiscard]] bool CheckExtensions(std::string_view extension) const
    {
        auto extensions = GetExtensions();
        return std::ranges::any_of(extensions, [extension](auto ext)
            {
                return extension == ext;
            });
    }
    virtual void DrawMainView() = 0;
    virtual void DrawInspector() = 0;
    // Return true if getting focused
    virtual bool DrawContentList(bool unfocus = false) = 0;
    virtual std::string_view GetSubFolder() = 0;
    virtual EditorType GetEditorType() = 0;
    virtual void Save() = 0;
    virtual void ReloadId() = 0;
    virtual void Delete() = 0;
    virtual void ImportResource(std::string_view path);
    [[nodiscard]] virtual std::span<const std::string_view> GetExtensions() const = 0;
};
}