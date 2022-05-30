#include "texture_editor.h"
#include "utils/log.h"
#include "editor.h"
#include "material_editor.h"
#include <imgui.h>
#include <fmt/format.h>
#include <array>
#include <fstream>

#include "engine/filesystem.h"

namespace gpr5300
{

void TextureEditor::DrawInspector()
{
    if (currentIndex_ >= textureInfos_.size())
    {
        return;
    }
    auto& currentTextureInfo = textureInfos_[currentIndex_];
    ImGui::Text("Filename: %s", currentTextureInfo.filename.c_str());

    if(GetFileExtension(currentTextureInfo.filename) == ".cube")
    {
        //choose cubemap textures
        static constexpr std::array<std::string_view, pb::Cubemap::LENGTH> cubemapSides
        {
            "Right Side",
            "Left Side",
            "Top Side",
            "Bottom Side",
            "Front Side",
            "Back Side",
        };
        ImGui::PushID("Cubemap textures");
        for(std::size_t i = 0; i < cubemapSides.size(); i++)
        {
            ImGui::PushID(i);
            if(currentTextureInfo.cubemap.texture_paths_size() <= i)
            {
                currentTextureInfo.cubemap.add_texture_paths();
            }
            auto* texturePath = currentTextureInfo.cubemap.mutable_texture_paths(i);
            if(ImGui::BeginCombo(cubemapSides[i].data(), texturePath->empty()? "No texture": texturePath->data()))
            {
                for(std::size_t textureIndex = 0; textureIndex < textureInfos_.size(); textureIndex++)
                {
                    auto& otherTexturePath = textureInfos_[textureIndex].info.path();
                    if(GetFileExtension(otherTexturePath) == ".cube")
                    {
                        continue;
                    }
                    const bool selected = !texturePath->empty() && fs::equivalent(*texturePath, otherTexturePath);
                    if(ImGui::Selectable(textureInfos_[textureIndex].filename.data(), selected))
                    {
                        *texturePath = otherTexturePath;
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopID();
        }
        ImGui::PopID();
    }

    static constexpr std::array<std::string_view, 4> wrappingModeNames
    {
        "REPEAT",
        "MIRROR_REPEAT",
        "CLAMP_TO_EDGE",
        "CLAMP_TO_BORDER"
    };
    if(ImGui::BeginCombo("Wrapping Mode", wrappingModeNames[currentTextureInfo.info.wrapping_mode()].data()))
    {
        for(std::size_t i = 0; i < wrappingModeNames.size(); ++i)
        {
            if(ImGui::Selectable(wrappingModeNames[i].data(), i == currentTextureInfo.info.wrapping_mode()))
            {
                currentTextureInfo.info.set_wrapping_mode(static_cast<pb::Texture_WrappingMode>(i));
            }
        }
        ImGui::EndCombo();
    }
    static constexpr std::array<std::string_view, 2> filterModeNames
    {
        "NEAREST",
        "LINEAR"
    };
    if (ImGui::BeginCombo("Filter Mode", filterModeNames[currentTextureInfo.info.filter_mode()].data()))
    {
        for (std::size_t i = 0; i < filterModeNames.size(); ++i)
        {
            if (ImGui::Selectable(filterModeNames[i].data(), i == currentTextureInfo.info.filter_mode()))
            {
                currentTextureInfo.info.set_filter_mode(static_cast<pb::Texture_FilteringMode>(i));
            }
        }
        ImGui::EndCombo();
    }

    bool generateMipMaps = currentTextureInfo.info.generate_mipmaps();
    if(ImGui::Checkbox("Generate Mip Map", &generateMipMaps))
    {
        currentTextureInfo.info.set_generate_mipmaps(generateMipMaps);
    }
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
        if(GetFileExtension(textureInfo.info.path()) == ".cube")
        {
            std::ofstream cubeOut(textureInfo.info.path(), std::ios::binary);
            if (!textureInfo.cubemap.SerializeToOstream(&cubeOut))
            {
                LogWarning(fmt::format("Could not save cubemap info at: {}", textureInfo.info.path()));
            }
        }
        std::ofstream fileOut(textureInfo.infoPath, std::ios::binary);
        if (!textureInfo.info.SerializeToOstream(&fileOut))
        {
            LogWarning(fmt::format("Could not save texture info at: {}", textureInfo.infoPath));
        }
    }
}

void TextureEditor::AddResource(const Resource &resource)
{
    auto& filesystem = FilesystemLocator::get();
    TextureInfo textureInfo{};
    textureInfo.resourceId = resource.resourceId;
    textureInfo.filename = GetFilename(resource.path);
    textureInfo.info.set_path(resource.path);
    textureInfo.infoPath = resource.path + ".meta";
    if(filesystem.FileExists(textureInfo.infoPath))
    {
        std::ifstream metaFile(textureInfo.infoPath,std::ios::binary);
        textureInfo.info.ParseFromIstream(&metaFile);
    }
    if(GetFileExtension(resource.path) == ".cube")
    {
        std::ifstream cubemapFile(resource.path, std::ios::binary);
        textureInfo.cubemap.ParseFromIstream(&cubemapFile);
    }
    textureInfos_.push_back(textureInfo);
}

void TextureEditor::RemoveResource(const Resource &resource)
{
    const auto it = std::ranges::find_if(textureInfos_, [&resource](const auto& texture)
    {
        return resource.resourceId == texture.resourceId;
    });
    if(it != textureInfos_.end())
    {
        textureInfos_.erase(it);
        const auto* editor = Editor::GetInstance();
        auto* materialEditor = dynamic_cast<MaterialEditor*>(editor->GetEditorSystem(EditorType::MATERIAL));

        materialEditor->RemoveResource(resource);
    }
}

void TextureEditor::UpdateExistingResource(const Resource &resource)
{

}

void TextureEditor::ReloadId()
{
}

TextureInfo* TextureEditor::GetTexture(ResourceId resourceId)
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

void TextureEditor::Delete()
{
    if (currentIndex_ >= textureInfos_.size())
    {
        return;
    }
    auto* editor = Editor::GetInstance();
    auto& resourceManager = editor->GetResourceManager();
    resourceManager.RemoveResource(textureInfos_[currentIndex_].info.path());
}

std::span<const std::string_view> TextureEditor::GetExtensions() const
{
    static constexpr std::array<std::string_view, 8> extensions
    {
        ".jpg",
        ".jpeg",
        ".png",
        ".bmp",
        ".tga",
        ".hdr",
        ".gif",
        ".cube"
    };
    return extensions;
}
}
