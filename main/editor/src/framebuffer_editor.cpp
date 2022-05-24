#include "framebuffer_editor.h"
#include <array>

namespace gpr5300
{
void FramebufferEditor::AddResource(const Resource& resource)
{
}

void FramebufferEditor::RemoveResource(const Resource& resource)
{
}

void FramebufferEditor::UpdateExistingResource(const Resource& resource)
{
}

void FramebufferEditor::DrawMainView()
{
}

void FramebufferEditor::DrawInspector()
{
}

bool FramebufferEditor::DrawContentList(bool unfocus)
{
    return false;
}

std::string_view FramebufferEditor::GetSubFolder()
{
    return "framebuffer/";
}

EditorType FramebufferEditor::GetEditorType()
{
    return EditorType::FRAMEBUFFER;
}

void FramebufferEditor::Save()
{
}

void FramebufferEditor::ReloadId()
{
}

void FramebufferEditor::Delete()
{
}

std::span<const std::string_view> FramebufferEditor::GetExtensions() const
{
    static constexpr std::array<std::string_view, 1> extensions = {
        ".framebuffer"
    };
    return extensions;
}
}
