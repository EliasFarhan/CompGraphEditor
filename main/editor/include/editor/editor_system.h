#pragma once
#include <string_view>
#include "resource.h"
#include <span>
#include <algorithm>

namespace editor
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
    FRAMEBUFFER,
    BUFFER,
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
    virtual void DrawInspector() = 0;
    virtual void DrawCenterView(){}
    // Return true if getting focused
    virtual bool DrawContentList(bool unfocus = false) = 0;
    virtual std::string_view GetSubFolder() = 0;
    virtual EditorType GetEditorType() = 0;
    virtual void Save(){}
    virtual void ReloadId() = 0;
    virtual void Delete() = 0;
    virtual void ImportResource(const core::Path &path);
    [[nodiscard]] virtual std::span<const std::string_view> GetExtensions() const = 0;
    virtual void Clear() = 0;
};
}