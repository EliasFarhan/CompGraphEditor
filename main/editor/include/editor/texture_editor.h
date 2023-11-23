#pragma once
#include "proto/renderer.pb.h"
#include "resource.h"
#include "editor_system.h"

namespace editor
{
struct KtxExportInfo
{
    int uastcLevel = 2; //(0-4)
    int quality = 128;
    int compLevel = 1;
    bool uastc = false;
    bool mipmap = false;
    bool srgb = false;
    bool compress = false;
};
struct TextureInfo
{
    std::string filename;
    std::string infoPath;
    core::pb::Texture info;
    ResourceId resourceId = INVALID_RESOURCE_ID;
    core::pb::Cubemap cubemap;
    core::TextureId textureId = core::INVALID_TEXTURE_ID;
    KtxExportInfo ktxInfo;
};


class TextureEditor final : public EditorSystem
{
public:
    void DrawInspector() override;

    bool DrawContentList(bool unfocus) override;

    void DrawCenterView() override;

    std::string_view GetSubFolder() override;

    EditorType GetEditorType() override;

    void Save() override;

    void AddResource(const Resource &resource) override;

    void RemoveResource(const Resource &resource) override;

    void UpdateExistingResource(const Resource &resource) override;
    void ReloadId() override;

    const auto& GetTextures() const { return textureInfos_; }
    TextureInfo* GetTexture(ResourceId resourceId);
    void Delete() override;
    std::span<const std::string_view> GetExtensions() const override;
    void Clear() override;
private:

    void CubeToKtx(const TextureInfo& textureInfo);
    void HdrToKtx(const TextureInfo& textureInfo);
    void ExportToKtx(const TextureInfo& textureInfo) const;
    std::vector<TextureInfo> textureInfos_;
    std::size_t currentIndex_ = -1;
};
}