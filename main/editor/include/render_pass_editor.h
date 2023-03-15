#pragma once

#include <string_view>

#include "editor_system.h"
#include "resource.h"
#include "proto/renderer.pb.h"

namespace editor
{


struct RenderPassInfo
{
    std::string path;
    std::string filename;
    core::pb::RenderPass info;
    ResourceId resourceId = INVALID_RESOURCE_ID;
};

class RenderPassEditor final : public EditorSystem
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
    const auto& GetRenderPasses() const { return renderPassInfos_; }
    const RenderPassInfo* GetRenderPass(ResourceId resourceId) const;
    void ReloadId() override;
    void Delete() override;
    std::span<const std::string_view> GetExtensions() const override;
private:
    std::vector<RenderPassInfo> renderPassInfos_;
    std::size_t currentIndex_ = -1;
};
}
