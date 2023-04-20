#pragma once
#include "proto/renderer.pb.h"
#include "resource.h"
#include "editor_system.h"

namespace editor
{

struct TextureInfo
{
    std::string filename;
    std::string infoPath;
    core::pb::Texture info;
    ResourceId resourceId = INVALID_RESOURCE_ID;
    core::pb::Cubemap cubemap;
};

class TextureEditor final : public EditorSystem
{
public:
    void DrawInspector() override;

    bool DrawContentList(bool unfocus) override;

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
    std::vector<TextureInfo> textureInfos_;
    std::size_t currentIndex_ = -1;
};
}