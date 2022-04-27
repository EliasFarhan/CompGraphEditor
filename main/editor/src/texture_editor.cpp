#include "texture_editor.h"
#include "utils/log.h"
#include <imgui.h>
#include <fmt/format.h>
#include <array>
#include <fstream>

namespace gpr5300
{

bool TextureEditor::CheckExtensions(std::string_view extension)
{
    static constexpr std::array<std::string_view, 7> extensions
    {
        ".jpg",
        ".jpeg",
        ".png",
        ".bmp",
        ".tga",
        ".hdr",
        ".gif"
    };
    return std::ranges::any_of(extensions, [extension](const auto imgExtension)
    {
            return extension == imgExtension;
    });
}

void TextureEditor::DrawMainView()
{

}

void TextureEditor::DrawInspector()
{
    if (currentIndex_ >= textureInfos_.size())
    {
        return;
    }
    const auto& currentTextureInfo = textureInfos_[currentIndex_];
    ImGui::Text("Filename: %s", currentTextureInfo.filename.c_str());
}

bool TextureEditor::DrawContentList(bool unfocus)
{
    bool wasFocused = false;
    if (unfocus)
        currentIndex_ = textureInfos_.size();
    for (std::size_t i = 0; i < textureInfos_.size(); i++)
    {
        const auto& shaderInfo = textureInfos_[i];
        if (ImGui::Selectable(shaderInfo.filename.data(), currentIndex_ == i))
        {
            currentIndex_ = i;
            wasFocused = true;
        }
    }
    return wasFocused;
}

std::string_view TextureEditor::GetSubFolder()
{
    return "textures/";
}

EditorType TextureEditor::GetEditorType()
{
    return EditorType::TEXTURE;
}

void TextureEditor::Save()
{
    for(auto& textureInfo : textureInfos_)
    {
        std::ofstream fileOut(textureInfo.infoPath, std::ios::binary);
        if (!textureInfo.info.SerializeToOstream(&fileOut))
        {
            LogWarning(fmt::format("Could not save texture info at: {}", textureInfo.infoPath));
        }
    }
}

void TextureEditor::AddResource(const Resource &resource)
{
    TextureInfo textureInfo{};
    textureInfo.resourceId = resource.resourceId;
    textureInfo.filename = GetFilename(resource.path);
    textureInfo.info.set_path(resource.path);
    textureInfo.infoPath = resource.path + ".meta";
    //TODO load texture with wrapping and mipmapping
    textureInfos_.push_back(textureInfo);
}

void TextureEditor::RemoveResource(const Resource &resource)
{

}

void TextureEditor::UpdateExistingResource(const Resource &resource)
{

}

void TextureEditor::ReloadId()
{
}

const TextureInfo* TextureEditor::GetTexture(ResourceId resourceId) const
{
    const auto it = std::ranges::find_if(textureInfos_, [resourceId](const auto& textureInfo)
        {
            return resourceId == textureInfo.resourceId;
        });
    if(it != textureInfos_.end())
    {
        return &*it;
    }
    return nullptr;
}
}
